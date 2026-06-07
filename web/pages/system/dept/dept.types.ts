/**
 * 部门管理类型定义
 */

import type { PageParams } from '@/utils/types';
import { createQueryKeys } from '@/utils/query';

// ============ QueryKeys ============

export const deptKeys = createQueryKeys('depts');

export const deptQueryKeys = {
    ...deptKeys,
    list: (params?: Dept.Query) => [...deptKeys.lists(), params] as const,
    tree: (status?: Dept.Status) => [...deptKeys.trees(), { status }] as const,
};

// ============ 枚举/状态类型 ============

export type DeptStatus = 'enabled' | 'disabled';

// ============ 列表项/详情类型 ============

export interface DeptItem {
    id: number;
    name: string;
    code?: string;
    parent_id?: number | null;
    sort_order: number;
    leader_id?: number | null;
    status: DeptStatus;
}

export interface DeptTreeItem extends DeptItem {
    children?: DeptTreeItem[];
}

// ============ 查询参数 ============

export interface DeptQuery extends PageParams {
    parent_id?: number | null;
    status?: DeptStatus;
}

// ============ DTO 类型 ============

export interface CreateDeptDto {
    name: string;
    code?: string;
    parent_id?: number | null;
    sort_order?: number;
    leader_id?: number | null;
    status?: DeptStatus;
}

export interface UpdateDeptDto {
    name?: string;
    code?: string;
    parent_id?: number | null;
    sort_order?: number;
    leader_id?: number | null;
    status?: DeptStatus;
}

export namespace Dept {
    export type Status = DeptStatus;
    export type Item = DeptItem;
    export type TreeItem = DeptTreeItem;
    export type Query = DeptQuery;
    export type CreateDto = CreateDeptDto;
    export type UpdateDto = UpdateDeptDto;
}
