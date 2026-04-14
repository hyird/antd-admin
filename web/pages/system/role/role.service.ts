/**
 * 角色管理 Service
 */

import { type UseQueryOptions, useQuery } from '@tanstack/react-query';
import type { Role } from './role.types';
import { roleQueryKeys } from './role.types';
import type { PaginatedResult } from '@/utils/types';
import { useMutationWithMessage, useSaveMutation } from '@/hooks/useMutation';
import { create, getAll, getDetail, getList, remove, update } from './role.api';

export { create, getAll, getDetail, getList, remove, update } from './role.api';

// ============ Queries ============

type RoleListResult = PaginatedResult<Role.Item>;

export function useRoleList(
    params?: Role.Query,
    options?: Omit<UseQueryOptions<RoleListResult>, 'queryKey' | 'queryFn'>
) {
    return useQuery({
        queryKey: roleQueryKeys.list(params),
        queryFn: () => getList(params),
        ...options,
    });
}

export function useRoleDetail(
    id: number,
    options?: Omit<UseQueryOptions<Role.Detail>, 'queryKey' | 'queryFn'>
) {
    return useQuery({
        queryKey: roleQueryKeys.detail(id),
        queryFn: () => getDetail(id),
        enabled: id > 0,
        ...options,
    });
}

export function useRoleOptions(
    options?: Omit<UseQueryOptions<Role.Option[]>, 'queryKey' | 'queryFn'>
) {
    return useQuery({
        queryKey: roleQueryKeys.options(),
        queryFn: getAll,
        staleTime: 5 * 60 * 1000,
        ...options,
    });
}

// ============ Mutations ============

export function useRoleUpdate() {
    return useMutationWithMessage({
        mutationFn: ({ id, data }: { id: number; data: Role.UpdateDto }) => update(id, data),
        successMessage: '更新成功',
        invalidateKeys: [roleQueryKeys.all],
    });
}

export function useRoleDelete() {
    return useMutationWithMessage({
        mutationFn: remove,
        successMessage: '删除成功',
        invalidateKeys: [roleQueryKeys.all],
    });
}

export function useRoleSave() {
    return useSaveMutation<Role.CreateDto & { id?: number }, Role.CreateDto, Role.UpdateDto>({
        createFn: create,
        updateFn: update,
        toUpdatePayload: ({ id: _id, ...data }: Role.CreateDto & { id?: number }) => data,
        createMessage: '保存成功',
        updateMessage: '保存成功',
        invalidateKeys: [roleQueryKeys.all],
    });
}
