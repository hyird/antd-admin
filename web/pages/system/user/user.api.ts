/**
 * 用户管理 API
 */

import type { User } from './user.types';
import { normalizePaginatedResponse } from '@/utils/pagination.response';
import type { PaginatedResponse } from '@/utils/pagination.types';
import { appendQueryParams } from '@/utils/query.params';
import request from '@/utils/http';

const ENDPOINTS = {
    BASE: '/api/users',
    DETAIL: (id: number) => `/api/users/${id}`,
    OPTIONS: '/api/users/options',
} as const;

export async function getList(params?: User.Query) {
    const response = await request.get<PaginatedResponse<User.Item>>(
        appendQueryParams(ENDPOINTS.BASE, params)
    );
    return normalizePaginatedResponse(response);
}

export function getDetail(id: number) {
    return request.get<User.Item>(ENDPOINTS.DETAIL(id));
}

export function getOptions(params?: Pick<User.Query, 'keyword'>) {
    return request.get<User.Option[]>(appendQueryParams(ENDPOINTS.OPTIONS, params));
}

export function create(data: User.CreateDto) {
    return request.post<void>(ENDPOINTS.BASE, data);
}

export function update(id: number, data: User.UpdateDto) {
    return request.put<void>(ENDPOINTS.DETAIL(id), data);
}

export function remove(id: number) {
    return request.delete<void>(ENDPOINTS.DETAIL(id));
}
