import { repo, AppDataSource } from '@/config/data';
import { Menu } from '@/modules/system/menu/menu.entity';
import type { LoginRequest, LoginResult, RefreshResult } from './auth.types';
import { comparePassword } from '@/utils/bcrypt';
import { signAccessToken, signRefreshToken, verifyRefreshToken } from '@/utils/jwt';
import { throwAppError } from '@/modules/common/http';
import { AuthError } from './auth.error';

import { IsNull } from 'typeorm';
import { rateLimitService } from './rate-limit.service';
import type { MenuItem } from '../menu/menu.types';
import type { RoleOption } from '../role/role.types';

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

        const tokenPayload = { userId: user.id, username: user.username };
        const token = signAccessToken(tokenPayload);
        const refreshToken = signRefreshToken(tokenPayload);
        const userInfo = await this.buildUserInfo(
            user.id,
            user.username,
            user.nickname,
            user.status
        );

        return { token, refreshToken, user: userInfo };
    }

    async refresh(refreshToken: string): Promise<RefreshResult> {
        if (!refreshToken) {
            throwAppError(AuthError.UNAUTHORIZED);
        }

        let payload: { userId: number; username: string };
        try {
            payload = verifyRefreshToken(refreshToken);
        } catch {
            throwAppError(AuthError.TOKEN_INVALID);
        }

        const user = await repo.user.findOne({
            where: { id: payload.userId, deleted_at: IsNull() },
        });

        if (!user) throwAppError(AuthError.USER_NOT_FOUND);
        if (user.status === 'disabled') throwAppError(AuthError.USER_DISABLED);

        const tokenPayload = { userId: user.id, username: user.username };
        const userInfo = await this.buildUserInfo(
            user.id,
            user.username,
            user.nickname,
            user.status
        );
        return {
            token: signAccessToken(tokenPayload),
            refreshToken: signRefreshToken(tokenPayload),
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
