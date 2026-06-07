import { createMiddleware } from 'hono/factory';
import { throwAppError } from '../common/http.js';
import { AuthError } from '../modules/system/auth/auth.error.js';
import { permissionService } from '../modules/system/auth/auth.service.js';
import type { AppEnv } from '../core/hono.env.js';

/**
 * 要求单个权限
 */
export function requirePermission(code: string) {
    return createMiddleware<AppEnv>(async (c, next) => {
        const jwt = c.get('jwt');
        if (!jwt?.user_id) {
            throwAppError(AuthError.UNAUTHORIZED);
        }
        const hasPermission = await permissionService.hasPermission(jwt.user_id, code);
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
        if (!jwt?.user_id) {
            throwAppError(AuthError.UNAUTHORIZED);
        }
        const hasPermission = await permissionService.hasAnyPermission(jwt.user_id, codes);
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
        if (!jwt?.user_id) {
            throwAppError(AuthError.UNAUTHORIZED);
        }
        const hasPermission = await permissionService.hasAllPermissions(jwt.user_id, codes);
        if (!hasPermission) {
            throwAppError(AuthError.PERMISSION_DENIED);
        }
        await next();
    });
}
