import { repo, AppDataSource } from '@/config/data';
import type { Menu } from '@/modules/system/menu/menu.entity';
import type { LoginRequest, LoginResult, RefreshResult } from './auth.types';
import { comparePassword } from '@/utils/bcrypt';
import { signAccessToken, signRefreshToken, verifyRefreshToken } from '@/utils/jwt';
import { throwAppError } from '@/common/http';
import { AuthError } from './auth.error';

import { IsNull } from 'typeorm';
import type { MenuItem } from '../menu/menu.types';
import type { RoleOption } from '../role/role.types';

/**
 * 登录限流服务（内存存储）
 * 记录登录失败次数，超过阈值后锁定账户
 */
interface FailureRecord {
    count: number;
    expiresAt: number;
}

class RateLimitService {
    private failureRecords: Map<string, FailureRecord> = new Map();
    private readonly maxAttempts: number;
    private readonly lockDurationMs: number;
    private cleanupInterval: NodeJS.Timeout | null = null;

    constructor(maxAttempts = 5, lockDurationMinutes = 15) {
        this.maxAttempts = maxAttempts;
        this.lockDurationMs = lockDurationMinutes * 60 * 1000;
        this.startCleanupTask();
    }

    private startCleanupTask(): void {
        this.cleanupInterval = setInterval(() => {
            const now = Date.now();
            for (const [key, record] of this.failureRecords.entries()) {
                if (record.expiresAt < now) {
                    this.failureRecords.delete(key);
                }
            }
        }, 60 * 1000);

        if (this.cleanupInterval.unref) {
            this.cleanupInterval.unref();
        }
    }

    stopCleanupTask(): void {
        if (this.cleanupInterval) {
            clearInterval(this.cleanupInterval);
            this.cleanupInterval = null;
        }
    }

    isLocked(username: string): boolean {
        const record = this.failureRecords.get(username);
        if (!record) return false;

        const now = Date.now();
        if (record.expiresAt < now) {
            this.failureRecords.delete(username);
            return false;
        }

        return record.count >= this.maxAttempts;
    }

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

    recordFailure(username: string): number {
        const now = Date.now();
        const record = this.failureRecords.get(username);

        if (!record || record.expiresAt < now) {
            this.failureRecords.set(username, {
                count: 1,
                expiresAt: now + this.lockDurationMs,
            });
            return 1;
        }

        record.count++;
        return record.count;
    }

    clearFailure(username: string): void {
        this.failureRecords.delete(username);
    }

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
}

/**
 * 权限服务
 * 管理用户权限缓存和校验
 */
interface UserPermissionCache {
    permissions: Set<string>;
    isSuperadmin: boolean;
    expireAt: number;
}

class PermissionService {
    private permissionCache = new Map<number, UserPermissionCache>();
    private CACHE_TTL = 60 * 1000;

    async getUserPermissions(
        userId: number
    ): Promise<{ permissions: Set<string>; isSuperadmin: boolean }> {
        const now = Date.now();
        const cached = this.permissionCache.get(userId);
        if (cached && cached.expireAt > now) {
            return {
                permissions: cached.permissions,
                isSuperadmin: cached.isSuperadmin,
            };
        }

        const user = await repo.user.findOne({
            where: { id: userId },
            relations: ['roles', 'roles.menus'],
        });

        if (!user) {
            return { permissions: new Set(), isSuperadmin: false };
        }

        const isSuperadmin =
            user.roles?.some((r) => r.code === 'superadmin' && r.status === 'enabled') ?? false;

        const permissions = new Set<string>();
        user.roles?.forEach((role) => {
            if (role.status === 'enabled') {
                role.menus?.forEach((menu) => {
                    if (menu.status === 'enabled' && menu.permission_code) {
                        permissions.add(menu.permission_code);
                    }
                });
            }
        });

        this.permissionCache.set(userId, {
            permissions,
            isSuperadmin,
            expireAt: now + this.CACHE_TTL,
        });

        return { permissions, isSuperadmin };
    }

    clearUserCache(userId: number): void {
        this.permissionCache.delete(userId);
    }

    clearAllCache(): void {
        this.permissionCache.clear();
    }

    async hasPermission(userId: number, code: string): Promise<boolean> {
        const { permissions, isSuperadmin } = await this.getUserPermissions(userId);
        return isSuperadmin || permissions.has(code);
    }

    async hasAnyPermission(userId: number, codes: string[]): Promise<boolean> {
        const { permissions, isSuperadmin } = await this.getUserPermissions(userId);
        return isSuperadmin || codes.some((code) => permissions.has(code));
    }

    async hasAllPermissions(userId: number, codes: string[]): Promise<boolean> {
        const { permissions, isSuperadmin } = await this.getUserPermissions(userId);
        return isSuperadmin || codes.every((code) => permissions.has(code));
    }
}

export const rateLimitService = new RateLimitService();
export const permissionService = new PermissionService();

interface RoleRow {
    id: number;
    name: string;
    code: string;
}

interface MenuRow {
    id: number;
    name: string;
    path: string | null;
    icon: string | null;
    parent_id: number | null;
    sort_order: number;
    type: string;
    component: string | null;
    status: string;
    permission_code: string | null;
}

export class AuthService {
    private toMenuItem = (m: Menu): MenuItem => ({
        id: m.id,
        name: m.name,
        path: m.path ?? undefined,
        icon: m.icon ?? undefined,
        parent_id: m.parent_id ?? undefined,
        sort_order: m.order,
        type: m.type,
        component: m.component ?? undefined,
        status: m.status,
        permission_code: m.permission_code ?? undefined,
        full_path: m.path ?? undefined,
    });

    // 获取用户的角色
    private async getUserRoles(userId: number): Promise<RoleOption[]> {
        const roles = (await AppDataSource.query(
            `SELECT r.id, r.name, r.code 
             FROM sys_role r 
             INNER JOIN sys_user_role ur ON r.id = ur.role_id 
             WHERE ur.user_id = ? AND r.deleted_at IS NULL`,
            [userId]
        )) as RoleRow[];
        return roles.map((r) => ({ id: r.id, name: r.name, code: r.code }));
    }

    private async buildUserInfo(
        userId: number,
        username: string,
        nickname: string | undefined,
        status: string
    ): Promise<LoginResult['user']> {
        const roles = await this.getUserRoles(userId);
        const isSuperadmin = roles.some((r) => r.code === 'superadmin');

        // 超级管理员不走权限裁剪，直接返回全量菜单。
        let menus: MenuItem[];
        if (isSuperadmin) {
            menus = await this.getAllMenus();
        } else {
            menus = await this.getUserRoleMenus(userId);
        }

        return {
            id: userId,
            username,
            nickname,
            status,
            roles,
            menus,
        };
    }

    private async getAllMenus(): Promise<MenuItem[]> {
        const allMenus = await repo.menu.find({
            where: { status: 'enabled', deleted_at: IsNull() },
            order: { order: 'ASC', id: 'ASC' },
        });
        return allMenus.map(this.toMenuItem);
    }

    private async getUserRoleMenus(userId: number): Promise<MenuItem[]> {
        const rows = (await AppDataSource.query(
            `SELECT DISTINCT
                m.id,
                m.name,
                m.path,
                m.icon,
                m.parent_id,
                m.\`order\` AS sort_order,
                m.type,
                m.component,
                m.status,
                m.permission_code
            FROM sys_menu m
            INNER JOIN sys_role_menu rm ON m.id = rm.menu_id
            INNER JOIN sys_user_role ur ON rm.role_id = ur.role_id
            INNER JOIN sys_role r ON ur.role_id = r.id
            WHERE ur.user_id = ?
              AND r.deleted_at IS NULL
              AND r.status = 'enabled'
              AND m.deleted_at IS NULL
              AND m.status = 'enabled'
            ORDER BY m.\`order\` ASC, m.id ASC`,
            [userId]
        )) as MenuRow[];

        return rows.map((row) => ({
            id: row.id,
            name: row.name,
            path: row.path ?? undefined,
            icon: row.icon ?? undefined,
            parent_id: row.parent_id ?? undefined,
            sort_order: row.sort_order,
            type: row.type as Menu['type'],
            component: row.component ?? undefined,
            status: row.status as Menu['status'],
            permission_code: row.permission_code ?? undefined,
            full_path: row.path ?? undefined,
        }));
    }

    async login({ username, password }: LoginRequest): Promise<LoginResult> {
        // 检查是否被锁定
        if (rateLimitService.isLocked(username)) {
            const remainingTime = rateLimitService.getRemainingLockTime(username);
            const minutes = Math.ceil(remainingTime / 60);
            throwAppError({
                code: 'TOO_MANY_ATTEMPTS',
                message: `登录失败次数过多，请${minutes}分钟后再试`,
                status: 429,
            });
        }

        const user = await repo.user.findOne({
            where: { username, deleted_at: IsNull() },
        });

        if (!user || !(await comparePassword(password, user.password_hash))) {
            // 记录登录失败
            const failureCount = rateLimitService.recordFailure(username);
            const remainingAttempts = 5 - failureCount;

            if (remainingAttempts > 0) {
                throwAppError({
                    code: 'PASSWORD_INCORRECT',
                    message: `用户名或密码错误，还剩${remainingAttempts}次尝试机会`,
                    status: 401,
                });
            } else {
                throwAppError({
                    code: 'TOO_MANY_ATTEMPTS',
                    message: '登录失败次数过多，账户已被锁定15分钟',
                    status: 429,
                });
            }
        }

        if (user.status === 'disabled') {
            throwAppError(AuthError.USER_DISABLED);
        }

        // 登录成功，清除失败记录
        rateLimitService.clearFailure(username);

        const tokenPayload = { user_id: user.id, username: user.username };
        const token = signAccessToken(tokenPayload);
        const refresh_token = signRefreshToken(tokenPayload);
        const userInfo = await this.buildUserInfo(
            user.id,
            user.username,
            user.nickname,
            user.status
        );

        return { token, refresh_token, user: userInfo };
    }

    async refresh(refresh_token: string): Promise<RefreshResult> {
        if (!refresh_token) {
            throwAppError(AuthError.UNAUTHORIZED);
        }

        let payload: { user_id: number; username: string };
        try {
            payload = verifyRefreshToken(refresh_token);
        } catch {
            throwAppError(AuthError.TOKEN_INVALID);
        }

        const user = await repo.user.findOne({
            where: { id: payload.user_id, deleted_at: IsNull() },
        });

        if (!user) throwAppError(AuthError.USER_NOT_FOUND);
        if (user.status === 'disabled') throwAppError(AuthError.USER_DISABLED);

        const tokenPayload = { user_id: user.id, username: user.username };
        const userInfo = await this.buildUserInfo(
            user.id,
            user.username,
            user.nickname,
            user.status
        );
        return {
            token: signAccessToken(tokenPayload),
            refresh_token: signRefreshToken(tokenPayload),
            user: userInfo,
        };
    }

    async logout(): Promise<void> {
        // 当前采用无状态 JWT，不维护服务端黑名单；前端清理本地状态即可完成登出。
        return;
    }

    async getCurrentUser(userId: number): Promise<LoginResult['user']> {
        const user = await repo.user.findOne({
            where: { id: userId, deleted_at: IsNull() },
        });

        if (!user) throwAppError(AuthError.USER_NOT_FOUND);
        if (user.status === 'disabled') throwAppError(AuthError.USER_DISABLED);

        return this.buildUserInfo(user.id, user.username, user.nickname, user.status);
    }
}

export const authService = new AuthService();
