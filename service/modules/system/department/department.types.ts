export type DepartmentStatus = 'enabled' | 'disabled';

export interface DepartmentItem {
    id: number;
    name: string;
    code?: string;
    parent_id?: number | null;
    sort_order: number;
    leader_id?: number | null;
    status: DepartmentStatus;
}

export interface DepartmentTreeItem extends DepartmentItem {
    children?: DepartmentTreeItem[];
}

export interface DepartmentQuery {
    status?: DepartmentStatus;
}

export interface CreateDepartmentDto {
    name: string;
    code?: string;
    parent_id?: number | null;
    sort_order?: number;
    leader_id?: number | null;
    status?: DepartmentStatus;
}

export interface UpdateDepartmentDto {
    name?: string;
    code?: string;
    parent_id?: number | null;
    sort_order?: number;
    leader_id?: number | null;
    status?: DepartmentStatus;
}
