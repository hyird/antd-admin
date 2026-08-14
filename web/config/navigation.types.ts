export type NavigationItemType = 'menu' | 'page' | 'button';

export interface NavigationItem {
    id: number;
    name: string;
    path?: string | null;
    component?: string;
    icon?: string;
    parent_id?: number | null;
    sort_order: number;
    type: NavigationItemType;
    status: 'enabled' | 'disabled';
    permission_code?: string;
}

export interface NavigationTreeItem extends NavigationItem {
    children?: NavigationTreeItem[];
    full_path?: string;
}
