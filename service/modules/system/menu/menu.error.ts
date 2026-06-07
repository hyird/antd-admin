// system/menu/menu.error.ts
import type { AppErrorDef } from '../../../common/http.js';

export const MenuError = {
    MENU_NOT_FOUND: {
        code: 'MENU_NOT_FOUND',
        message: '菜单不存在',
        status: 404,
    },
    MENU_PARENT_SELF: {
        code: 'MENU_PARENT_SELF',
        message: '父级菜单不能是自己',
        status: 400,
    },
    MENU_HAS_CHILDREN: {
        code: 'MENU_HAS_CHILDREN',
        message: '存在子菜单，不能删除',
        status: 400,
    },
    MENU_PATH_EXISTS: {
        code: 'MENU_PATH_EXISTS',
        message: '同级路由已存在',
        status: 400,
    },
    MENU_PARENT_NOT_FOUND: {
        code: 'MENU_PARENT_NOT_FOUND',
        message: '父级菜单不存在',
        status: 400,
    },
    MENU_TYPE_INVALID: {
        code: 'MENU_TYPE_INVALID',
        message: '菜单节点下只能挂菜单或页面',
        status: 400,
    },
    DEFAULT_MUST_BE_PAGE: {
        code: 'DEFAULT_MUST_BE_PAGE',
        message: '只有页面类型才能设为默认页面',
        status: 400,
    },
} as const satisfies Record<string, AppErrorDef>;
