/**
 * 角色管理类型定义
 */

import type { PageParams } from '@/utils/types';
import type { Menu } from '../menu/menu.types';
import { createQueryKeys } from '@/utils/query';

// ============ QueryKeys ============

export const roleKeys = createQueryKeys('roles');

export const roleQueryKeys = {
    ...roleKeys,
    list: (params?: Role.Query) => [...roleKeys.lists(), params] as const,
};

// ============ 枚举/状态类型 ============

export type RoleStatus = 'enabled' | 'disabled';

// ============ 列表项/详情类型 ============

export interface RoleItem {
    id: number;
    name: string;
    code: string;
    status: RoleStatus;
    menu_ids?: number[];
}

export interface RoleDetail extends RoleItem {
    menu_ids: number[];
    menus: {
        id: number;
        name: string;
        type: Menu.Type;
        parent_id: number | null;
    }[];
}

export interface RoleOption {
    id: number;
    name: string;
    code: string;
}

// ============ 查询参数 ============

export interface RoleQuery extends PageParams {
    status?: RoleStatus;
}

// ============ DTO 类型 ============

export interface CreateRoleDto {
    name: string;
    code: string;
    status?: RoleStatus;
    menu_ids?: number[];
}

export interface UpdateRoleDto {
    name?: string;
    code?: string;
    status?: RoleStatus;
    menu_ids?: number[];
}

export namespace Role {
    export type Status = RoleStatus;
    export type Item = RoleItem;
    export type Detail = RoleDetail;
    export type Option = RoleOption;
    export type Query = RoleQuery;
    export type CreateDto = CreateRoleDto;
    export type UpdateDto = UpdateRoleDto;
}
