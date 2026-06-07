import type { PageParams } from '../../../common/types.js';
import type { MenuType } from '../menu/menu.types';

export type RoleStatus = 'enabled' | 'disabled';

export interface RoleItem {
    id: number;
    name: string;
    code: string;
    status: RoleStatus;
    menu_ids: number[];
}

export interface RoleDetail extends RoleItem {
    menus: {
        id: number;
        name: string;
        type: MenuType;
        parent_id: number | null;
    }[];
}

export interface RoleOption {
    id: number;
    name: string;
    code: string;
}

export interface RoleQuery extends PageParams {
    status?: RoleStatus;
}

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
