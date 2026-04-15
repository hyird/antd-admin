// system/user/user.error.ts
import type { AppErrorDef } from '@/common/http';

export const UserError = {
    USERNAME_EXISTS: {
        code: 'USERNAME_EXISTS',
        message: '用户名已存在',
        status: 400,
    },
    USER_NOT_FOUND: {
        code: 'USER_NOT_FOUND',
        message: '用户不存在',
        status: 404,
    },
    PHONE_EXISTS: {
        code: 'PHONE_EXISTS',
        message: '手机号已被使用',
        status: 400,
    },
    EMAIL_EXISTS: {
        code: 'EMAIL_EXISTS',
        message: '邮箱已被使用',
        status: 400,
    },
    PHONE_INVALID: {
        code: 'PHONE_INVALID',
        message: '手机号格式不正确',
        status: 400,
    },
    EMAIL_INVALID: {
        code: 'EMAIL_INVALID',
        message: '邮箱格式不正确',
        status: 400,
    },
    ROLE_REQUIRED: {
        code: 'ROLE_REQUIRED',
        message: '用户必须分配至少一个角色',
        status: 400,
    },
    ADMIN_ROLE_PROTECTED: {
        code: 'ADMIN_ROLE_PROTECTED',
        message: '内置管理员账户的角色不可修改',
        status: 400,
    },
    ADMIN_DELETE_PROTECTED: {
        code: 'ADMIN_DELETE_PROTECTED',
        message: '内置管理员账户不可删除',
        status: 400,
    },
} as const satisfies Record<string, AppErrorDef>;
