/**
 * 菜单管理 API
 */

import type { Menu } from './menu.types';
import { appendQueryParams } from '@/utils/query';
import request from '@/utils/http';

/** API 端点 */
const ENDPOINTS = {
    BASE: '/api/menus',
    DETAIL: (id: number) => `/api/menus/${id}`,
    TREE: '/api/menus/tree',
    BATCH_BUTTONS: '/api/menus/batch-buttons',
} as const;

/** 获取菜单列表 */
export function getList(params?: Menu.Query) {
    return request.get<Menu.Item[]>(appendQueryParams(ENDPOINTS.BASE, params));
}

/** 获取菜单树 */
export function getTree(status?: Menu.Status) {
    return request.get<Menu.TreeItem[]>(appendQueryParams(ENDPOINTS.TREE, status ? { status } : undefined));
}

/** 获取菜单详情 */
export function getDetail(id: number) {
    return request.get<Menu.Item>(ENDPOINTS.DETAIL(id));
}

/** 创建菜单 */
export function create(data: Menu.CreateDto) {
    return request.post<void>(ENDPOINTS.BASE, data);
}

/** 更新菜单 */
export function update(id: number, data: Menu.UpdateDto) {
    return request.put<void>(ENDPOINTS.DETAIL(id), data);
}

/** 删除菜单 */
export function remove(id: number) {
    return request.delete<void>(ENDPOINTS.DETAIL(id));
}

/** 菜单排序 */
export function reorder(data: Menu.ReorderDto) {
    return request.post<void>(`${ENDPOINTS.BASE}/reorder`, data);
}

/** 批量新增页面按钮权限 */
export function batchCreateButtons(data: Menu.BatchCreateButtonsDto) {
    return request.post<{ created_count: number }>(ENDPOINTS.BATCH_BUTTONS, data);
}
