/**
 * 角色管理 API
 */

import type { Role } from './role.types';
import { normalizePaginatedResponse } from '@/utils/pagination.response';
import type { PaginatedResponse } from '@/utils/pagination.types';
import { appendQueryParams } from '@/utils/query.params';
import request from '@/utils/http';

const ENDPOINTS = {
    BASE: '/api/roles',
    DETAIL: (id: number) => `/api/roles/${id}`,
    ALL: '/api/roles/all',
} as const;

export async function getList(params?: Role.Query) {
    const response = await request.get<PaginatedResponse<Role.Item>>(
        appendQueryParams(ENDPOINTS.BASE, params)
    );
    return normalizePaginatedResponse(response);
}

export function getDetail(id: number) {
    return request.get<Role.Detail>(ENDPOINTS.DETAIL(id));
}

export function getAll() {
    return request.get<Role.Option[]>(ENDPOINTS.ALL);
}

export function create(data: Role.CreateDto) {
    return request.post<void>(ENDPOINTS.BASE, data);
}

export function update(id: number, data: Role.UpdateDto) {
    return request.put<void>(ENDPOINTS.DETAIL(id), data);
}

export function remove(id: number) {
    return request.delete<void>(ENDPOINTS.DETAIL(id));
}
