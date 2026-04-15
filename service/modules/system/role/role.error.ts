// system/role/role.error.ts
import type { AppErrorDef } from '@/common/http';

export const RoleError = {
    CODE_EXISTS: {
        code: 'ROLE_CODE_EXISTS',
        message: '角色编码已存在',
        status: 400,
    },
    NOT_FOUND: {
        code: 'ROLE_NOT_FOUND',
        message: '角色不存在',
        status: 404,
    },
    SUPERADMIN_CANNOT_DELETE: {
        code: 'ROLE_CANNOT_DELETE',
        message: '超级管理员角色不可删除',
        status: 400,
    },
    SUPERADMIN_CANNOT_MODIFY: {
        code: 'ROLE_CANNOT_MODIFY',
        message: '超级管理员角色编码不可修改',
        status: 400,
    },
    HAS_USERS: {
        code: 'ROLE_HAS_USERS',
        message: '该角色下存在用户，不可删除',
        status: 400,
    },
} as const satisfies Record<string, AppErrorDef>;
