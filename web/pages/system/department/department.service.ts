/**
 * 部门管理 Service
 */

import { type UseQueryOptions, useQuery } from '@tanstack/react-query';
import type { TreeSelectProps } from 'antd';
import { useMemo } from 'react';
import { useSaveMutation } from '@/hooks/useMutation';
import type { Department } from './department.types';
import { create, getDetail, getList, getTree, remove, update } from './department.api';
import { departmentQueryKeys } from './department.types';

export { create, getDetail, getList, getTree, remove, update } from './department.api';

export function useDepartmentList(
    params?: Department.Query,
    options?: Omit<UseQueryOptions<Department.Item[]>, 'queryKey' | 'queryFn'>
) {
    return useQuery({
        queryKey: departmentQueryKeys.list(params),
        queryFn: () => getList(params),
        ...options,
    });
}

export function useDepartmentTree(
    status?: Department.Status,
    options?: Omit<UseQueryOptions<Department.TreeItem[]>, 'queryKey' | 'queryFn'>
) {
    return useQuery({
        queryKey: departmentQueryKeys.tree(status),
        queryFn: () => getTree(status),
        staleTime: 5 * 60 * 1000,
        ...options,
    });
}

export function useDepartmentDetail(
    id: number,
    options?: Omit<UseQueryOptions<Department.Item>, 'queryKey' | 'queryFn'>
) {
    return useQuery({
        queryKey: departmentQueryKeys.detail(id),
        queryFn: () => getDetail(id),
        enabled: !!id,
        ...options,
    });
}

export function useDepartmentTreeSelect(status?: Department.Status) {
    const { data: tree = [], ...rest } = useDepartmentTree(status);

    const treeData = useMemo((): TreeSelectProps['treeData'] => {
        const loop = (nodes: Department.TreeItem[]): TreeSelectProps['treeData'] =>
            nodes.map((n) => ({
                title: n.name,
                value: n.id,
                children: n.children?.length ? loop(n.children) : undefined,
            }));
        return loop(tree);
    }, [tree]);

    const departmentMap = useMemo(() => {
        const map = new Map<number, Department.TreeItem>();
        const flatten = (nodes: Department.TreeItem[]) => {
            nodes.forEach((n) => {
                map.set(n.id, n);
                if (n.children?.length) flatten(n.children);
            });
        };
        flatten(tree);
        return map;
    }, [tree]);

    return {
        ...rest,
        tree,
        treeData,
        departmentMap,
    };
}

export function useDepartmentSave() {
    return useSaveMutation<
        Department.CreateDto & { id?: number },
        Department.CreateDto,
        Department.UpdateDto
    >({
        createFn: create,
        updateFn: update,
        toUpdatePayload: ({ id: _id, ...data }) => data,
        createMessage: '保存成功',
        updateMessage: '保存成功',
        invalidateKeys: [departmentQueryKeys.all],
    });
}
