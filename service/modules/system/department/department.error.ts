// src/modules/department/department.error.ts
import type { AppErrorDef } from '../../../common/http.js';

export const DepartmentError = {
    NOT_FOUND: {
        code: 'DEPARTMENT_NOT_FOUND',
        message: '部门不存在',
        status: 404,
    },
    PARENT_SELF: {
        code: 'DEPARTMENT_PARENT_SELF',
        message: '父级部门不能是自己',
        status: 400,
    },
    PARENT_IS_CHILD: {
        code: 'DEPARTMENT_PARENT_IS_CHILD',
        message: '父级部门不能是自己的子部门',
        status: 400,
    },
    HAS_CHILDREN: {
        code: 'DEPARTMENT_HAS_CHILDREN',
        message: '存在子部门，不能删除',
        status: 400,
    },
    HAS_USERS: {
        code: 'DEPARTMENT_HAS_USERS',
        message: '部门下存在用户，不能删除',
        status: 400,
    },
    CODE_EXISTS: {
        code: 'DEPARTMENT_CODE_EXISTS',
        message: '部门编码已存在',
        status: 400,
    },
} as const satisfies Record<string, AppErrorDef>;
