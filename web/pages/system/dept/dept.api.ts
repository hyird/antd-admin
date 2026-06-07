/**
 * 部门管理 API
 */

import type { Dept } from './dept.types';
import { appendQueryParams } from '@/utils/query';
import request from '@/utils/http';

/** API 端点 */
const ENDPOINTS = {
    BASE: '/api/depts',
    DETAIL: (id: number) => `/api/depts/${id}`,
    TREE: '/api/depts/tree',
} as const;

/** 获取部门列表 */
export function getList(params?: Dept.Query) {
    return request.get<Dept.Item[]>(appendQueryParams(ENDPOINTS.BASE, params));
}

/** 获取部门树 */
export function getTree(status?: Dept.Status) {
    return request.get<Dept.TreeItem[]>(
        appendQueryParams(ENDPOINTS.TREE, status ? { status } : undefined)
    );
}

/** 获取部门详情 */
export function getDetail(id: number) {
    return request.get<Dept.Item>(ENDPOINTS.DETAIL(id));
}

/** 创建部门 */
export function create(data: Dept.CreateDto) {
    return request.post<void>(ENDPOINTS.BASE, data);
}

/** 更新部门 */
export function update(id: number, data: Dept.UpdateDto) {
    return request.put<void>(ENDPOINTS.DETAIL(id), data);
}

/** 删除部门 */
export function remove(id: number) {
    return request.delete<void>(ENDPOINTS.DETAIL(id));
}
