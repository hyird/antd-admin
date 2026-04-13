/**
 * 用户管理 Service
 */

import { type UseQueryOptions, useQuery } from '@tanstack/react-query';
import type { User } from './user.types';
import { userQueryKeys } from './user.types';
import type { PaginatedResult } from '@/utils/types';
import { useMutationWithMessage, useSaveMutation } from '@/hooks/useMutation';

// ============ API ============

import request from '@/utils/http';

const ENDPOINTS = {
    BASE: '/api/users',
    DETAIL: (id: number) => `/api/users/${id}`,
    OPTIONS: '/api/users/options',
} as const;

export function getList(params?: User.Query) {
    return request.get<PaginatedResult<User.Item>>(ENDPOINTS.BASE, { params });
}

export function getDetail(id: number) {
    return request.get<User.Item>(ENDPOINTS.DETAIL(id));
}

export function getOptions(params?: Pick<User.Query, 'keyword'>) {
    return request.get<User.Option[]>(ENDPOINTS.OPTIONS, { params });
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

// ============ Queries ============

type UserListResult = PaginatedResult<User.Item>;

export function useUserList(
    params?: User.Query,
    options?: Omit<UseQueryOptions<UserListResult>, 'queryKey' | 'queryFn'>
) {
    return useQuery({
        queryKey: userQueryKeys.list(params),
        queryFn: () => getList(params),
        ...options,
    });
}

export function useUserDetail(
    id: number,
    options?: Omit<UseQueryOptions<User.Item>, 'queryKey' | 'queryFn'>
) {
    return useQuery({
        queryKey: userQueryKeys.detail(id),
        queryFn: () => getDetail(id),
        enabled: !!id,
        ...options,
    });
}

export function useUserOptions(
    options?: Omit<UseQueryOptions<User.Option[]>, 'queryKey' | 'queryFn'>
) {
    return useQuery({
        queryKey: userQueryKeys.options(),
        queryFn: () => getOptions(),
        staleTime: 5 * 60 * 1000,
        ...options,
    });
}

// ============ Mutations ============

export function useUserDelete() {
    return useMutationWithMessage({
        mutationFn: remove,
        successMessage: '删除成功',
        invalidateKeys: [userQueryKeys.all],
    });
}

export function useUserSave() {
    return useSaveMutation<User.CreateDto & { id?: number }, User.CreateDto, User.UpdateDto>({
        createFn: create,
        updateFn: update,
        toUpdatePayload: ({
            id: _id,
            username: _username,
            ...data
        }: User.CreateDto & { id?: number }) => data as User.UpdateDto,
        createMessage: '保存成功',
        updateMessage: '保存成功',
        invalidateKeys: [userQueryKeys.all],
    });
}
