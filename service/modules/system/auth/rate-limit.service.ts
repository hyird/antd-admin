/**
 * 登录限流服务（内存存储）
 * 记录登录失败次数，超过阈值后锁定账户
 */

interface FailureRecord {
    count: number;
    expiresAt: number; // 时间戳（毫秒）
}

export class RateLimitService {
    private failureRecords: Map<string, FailureRecord> = new Map();
    private readonly maxAttempts: number;
    private readonly lockDurationMs: number;
    private cleanupInterval: NodeJS.Timeout | null = null;

    constructor(maxAttempts = 5, lockDurationMinutes = 15) {
        this.maxAttempts = maxAttempts;
        this.lockDurationMs = lockDurationMinutes * 60 * 1000;
        this.startCleanupTask();
    }

    /**
     * 启动定时清理任务（每分钟清理过期记录）
     */
    private startCleanupTask(): void {
        this.cleanupInterval = setInterval(() => {
            const now = Date.now();
            for (const [key, record] of this.failureRecords.entries()) {
                if (record.expiresAt < now) {
                    this.failureRecords.delete(key);
                }
            }
        }, 60 * 1000); // 每分钟执行一次

        // 确保进程退出时清理定时器
        if (this.cleanupInterval.unref) {
            this.cleanupInterval.unref();
        }
    }

    /**
     * 停止清理任务（用于测试或优雅关闭）
     */
    stopCleanupTask(): void {
        if (this.cleanupInterval) {
            clearInterval(this.cleanupInterval);
            this.cleanupInterval = null;
        }
    }

    /**
     * 检查是否被锁定
     */
    isLocked(username: string): boolean {
        const record = this.failureRecords.get(username);
        if (!record) return false;

        const now = Date.now();
        if (record.expiresAt < now) {
            // 记录已过期，删除并返回未锁定
            this.failureRecords.delete(username);
            return false;
        }

        return record.count >= this.maxAttempts;
    }

    /**
     * 获取失败次数
     */
    getFailureCount(username: string): number {
        const record = this.failureRecords.get(username);
        if (!record) return 0;

        const now = Date.now();
        if (record.expiresAt < now) {
            this.failureRecords.delete(username);
            return 0;
        }

        return record.count;
    }

    /**
     * 记录登录失败
     */
    recordFailure(username: string): number {
        const now = Date.now();
        const record = this.failureRecords.get(username);

        if (!record || record.expiresAt < now) {
            // 创建新记录或更新过期记录
            this.failureRecords.set(username, {
                count: 1,
                expiresAt: now + this.lockDurationMs,
            });
            return 1;
        }

        // 增加失败次数
        record.count++;
        return record.count;
    }

    /**
     * 清除失败记录（登录成功后调用）
     */
    clearFailure(username: string): void {
        this.failureRecords.delete(username);
    }

    /**
     * 获取剩余锁定时间（秒）
     */
    getRemainingLockTime(username: string): number {
        const record = this.failureRecords.get(username);
        if (!record) return 0;

        const now = Date.now();
        if (record.expiresAt < now) {
            this.failureRecords.delete(username);
            return 0;
        }

        if (record.count < this.maxAttempts) {
            return 0;
        }

        return Math.ceil((record.expiresAt - now) / 1000);
    }

    /**
     * 获取统计信息（用于调试）
     */
    getStats(): {
        totalRecords: number;
        lockedUsers: number;
        maxAttempts: number;
        lockDurationMinutes: number;
    } {
        const now = Date.now();
        let lockedUsers = 0;

        for (const record of this.failureRecords.values()) {
            if (record.expiresAt >= now && record.count >= this.maxAttempts) {
                lockedUsers++;
            }
        }

        return {
            totalRecords: this.failureRecords.size,
            lockedUsers,
            maxAttempts: this.maxAttempts,
            lockDurationMinutes: this.lockDurationMs / (60 * 1000),
        };
    }
}

// 单例实例
export const rateLimitService = new RateLimitService();
