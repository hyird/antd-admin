/**
 * 部门管理类型定义
 */

import { createQueryKeys } from '@/utils/query';

// ============ QueryKeys ============

export const departmentKeys = createQueryKeys('departments');

export const departmentQueryKeys = {
    ...departmentKeys,
    list: (params?: Department.Query) => [...departmentKeys.lists(), params] as const,
    tree: (status?: Department.Status) => [...departmentKeys.trees(), { status }] as const,
};

// ============ 枚举/状态类型 ============

export type DepartmentStatus = 'enabled' | 'disabled';

// ============ 列表项/详情类型 ============

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

// ============ 查询参数 ============

export interface DepartmentQuery {
    keyword?: string;
    status?: DepartmentStatus;
}

// ============ DTO 类型 ============

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

export namespace Department {
    export type Status = DepartmentStatus;
    export type Item = DepartmentItem;
    export type TreeItem = DepartmentTreeItem;
    export type Query = DepartmentQuery;
    export type CreateDto = CreateDepartmentDto;
    export type UpdateDto = UpdateDepartmentDto;
}
