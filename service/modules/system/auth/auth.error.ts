// system/auth/auth.error.ts
import type { AppErrorDef } from '../../../common/http.js';

export const AuthError = {
    USER_NOT_FOUND: {
        code: 'USER_NOT_FOUND',
        message: '用户不存在',
        status: 400,
    },
    USER_DISABLED: {
        code: 'USER_DISABLED',
        message: '用户已被禁用',
        status: 403,
    },
    PASSWORD_INCORRECT: {
        code: 'PASSWORD_INCORRECT',
        message: '用户名或密码错误',
        status: 400,
    },
    UNAUTHORIZED: {
        code: 'UNAUTHORIZED',
        message: '未登录',
        status: 401,
    },
    TOKEN_EXPIRED: {
        code: 'TOKEN_EXPIRED',
        message: 'Token已过期',
        status: 401,
    },
    TOKEN_INVALID: {
        code: 'TOKEN_INVALID',
        message: 'Token无效',
        status: 401,
    },
    PERMISSION_DENIED: {
        code: 'PERMISSION_DENIED',
        message: '无权限',
        status: 403,
    },
} as const satisfies Record<string, AppErrorDef>;
