export type MenuType = 'menu' | 'page' | 'button';
export type MenuStatus = 'enabled' | 'disabled';

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
    full_path?: string;
}

export interface MenuTreeItem extends MenuItem {
    children?: MenuTreeItem[];
}

export interface MenuQuery {
    status?: MenuStatus;
}

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
