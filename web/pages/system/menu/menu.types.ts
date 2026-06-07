/**
 * 菜单管理类型定义
 */

import type { PageParams } from '@/utils/types';
import { createQueryKeys } from '@/utils/query';

// ============ QueryKeys ============

export const menuKeys = createQueryKeys('menus');

export const menuQueryKeys = {
    ...menuKeys,
    list: (params?: Menu.Query) => [...menuKeys.lists(), params] as const,
    tree: (status?: Menu.Status) => [...menuKeys.trees(), { status }] as const,
};

// ============ 枚举/状态类型 ============

export type MenuType = 'menu' | 'page' | 'button';
export type MenuStatus = 'enabled' | 'disabled';

// ============ 列表项/详情类型 ============

export interface MenuItem {
    id: number;
    name: string;
    path?: string | null;
    component?: string;
    icon?: string;
    parent_id?: number | null;
    sort_order: number;
    type: MenuType;
    status: MenuStatus;
    permission_code?: string;
}

export interface MenuTreeItem extends MenuItem {
    children?: MenuTreeItem[];
    full_path?: string;
}

// ============ 查询参数 ============

export interface MenuQuery extends PageParams {
    parent_id?: number | null;
    status?: MenuStatus;
}

// ============ DTO 类型 ============

export interface CreateMenuDto {
    name: string;
    path?: string | null;
    icon?: string;
    component?: string;
    parent_id?: number | null;
    sort_order?: number;
    type?: MenuType;
    status?: MenuStatus;
    permission_code?: string;
    is_default?: boolean;
}

export interface UpdateMenuDto {
    name?: string;
    path?: string | null;
    icon?: string;
    component?: string;
    parent_id?: number | null;
    sort_order?: number;
    type?: MenuType;
    status?: MenuStatus;
    permission_code?: string;
    is_default?: boolean;
}

export interface ReorderMenuItemDto {
    id: number;
    sort_order: number;
    parent_id?: number | null;
}

export interface ReorderMenuDto {
    items: ReorderMenuItemDto[];
}

export interface BatchCreateMenuButtonItemDto {
    name: string;
    permission_code: string;
}

export interface BatchCreateMenuButtonsDto {
    parent_id: number;
    items: BatchCreateMenuButtonItemDto[];
}

export namespace Menu {
    export type Type = MenuType;
    export type Status = MenuStatus;
    export type Item = MenuItem;
    export type TreeItem = MenuTreeItem;
    export type Query = MenuQuery;
    export type CreateDto = CreateMenuDto;
    export type UpdateDto = UpdateMenuDto;
    export type ReorderItemDto = ReorderMenuItemDto;
    export type ReorderDto = ReorderMenuDto;
    export type BatchCreateButtonItemDto = BatchCreateMenuButtonItemDto;
    export type BatchCreateButtonsDto = BatchCreateMenuButtonsDto;
}

// ============ 常量 ============

export const MenuTypeMap: Record<MenuType, { text: string; color: string }> = {
    menu: { text: '菜单', color: 'blue' },
    page: { text: '页面', color: 'green' },
    button: { text: '按钮', color: 'purple' },
};
