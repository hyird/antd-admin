import { createMiddleware } from 'hono/factory';
import { throwAppError } from '@/modules/common/http';
import { AuthError } from './auth.error';
import { permissionService } from './permission.service';
import { AppEnv } from '@/core/hono.env';

/**
 * 要求单个权限
 */
export function requirePermission(code: string) {
    return createMiddleware<AppEnv>(async (c, next) => {
        const jwt = c.get('jwt');
        if (!jwt?.userId) {
            throwAppError(AuthError.UNAUTHORIZED);
        }
        const hasPermission = await permissionService.hasPermission(jwt.userId, code);
        if (!hasPermission) {
            throwAppError(AuthError.PERMISSION_DENIED);
        }
        await next();
    });
}

/**
 * 要求任意一个权限（满足其一即可）
 */
export function requireAnyPermission(codes: string[]) {
    return createMiddleware<AppEnv>(async (c, next) => {
        const jwt = c.get('jwt');
        if (!jwt?.userId) {
            throwAppError(AuthError.UNAUTHORIZED);
        }
        const hasPermission = await permissionService.hasAnyPermission(jwt.userId, codes);
        if (!hasPermission) {
            throwAppError(AuthError.PERMISSION_DENIED);
        }
        await next();
    });
}

/**
 * 要求所有权限（必须全部满足）
 */
export function requireAllPermissions(codes: string[]) {
    return createMiddleware<AppEnv>(async (c, next) => {
        const jwt = c.get('jwt');
        if (!jwt?.userId) {
            throwAppError(AuthError.UNAUTHORIZED);
        }
        const hasPermission = await permissionService.hasAllPermissions(jwt.userId, codes);
        if (!hasPermission) {
            throwAppError(AuthError.PERMISSION_DENIED);
        }
        await next();
    });
}
