/**
 * 部门管理 Service
 */

import { type UseQueryOptions, useQuery } from '@tanstack/react-query';
import type { TreeSelectProps } from 'antd';
import { useMemo } from 'react';
import { useSaveMutation } from '@/hooks/useMutation';
import type { Dept } from './dept.types';
import { create, getDetail, getList, getTree, update } from './dept.api';
import { deptQueryKeys } from './dept.types';

export { create, getDetail, getList, getTree, remove, update } from './dept.api';

export function useDeptList(
    params?: Dept.Query,
    options?: Omit<UseQueryOptions<Dept.Item[]>, 'queryKey' | 'queryFn'>
) {
    return useQuery({
        queryKey: deptQueryKeys.list(params),
        queryFn: () => getList(params),
        ...options,
    });
}

export function useDeptTree(
    status?: Dept.Status,
    options?: Omit<UseQueryOptions<Dept.TreeItem[]>, 'queryKey' | 'queryFn'>
) {
    return useQuery({
        queryKey: deptQueryKeys.tree(status),
        queryFn: () => getTree(status),
        staleTime: 5 * 60 * 1000,
        ...options,
    });
}

export function useDeptDetail(
    id: number,
    options?: Omit<UseQueryOptions<Dept.Item>, 'queryKey' | 'queryFn'>
) {
    return useQuery({
        queryKey: deptQueryKeys.detail(id),
        queryFn: () => getDetail(id),
        enabled: !!id,
        ...options,
    });
}

export function useDeptTreeSelect(status?: Dept.Status) {
    const { data: tree = [], ...rest } = useDeptTree(status);

    const treeData = useMemo((): TreeSelectProps['treeData'] => {
        const loop = (nodes: Dept.TreeItem[]): TreeSelectProps['treeData'] =>
            nodes.map((n) => ({
                title: n.name,
                value: n.id,
                children: n.children?.length ? loop(n.children) : undefined,
            }));
        return loop(tree);
    }, [tree]);

    const deptMap = useMemo(() => {
        const map = new Map<number, Dept.TreeItem>();
        const flatten = (nodes: Dept.TreeItem[]) => {
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
        deptMap,
    };
}

export function useDeptSave() {
    return useSaveMutation<Dept.CreateDto & { id?: number }, Dept.CreateDto, Dept.UpdateDto>({
        createFn: create,
        updateFn: update,
        toUpdatePayload: ({ id: _id, ...data }) => data,
        createMessage: '保存成功',
        updateMessage: '保存成功',
        invalidateKeys: [deptQueryKeys.all],
    });
}
