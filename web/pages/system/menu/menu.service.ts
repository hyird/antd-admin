/**
 * 菜单管理 Service
 */

import { type UseQueryOptions, useQuery } from '@tanstack/react-query';
import type { TreeSelectProps } from 'antd';
import { useMemo } from 'react';
import { useSaveMutation } from '@/hooks/useMutation';
import type { PaginatedResult } from '@/utils/types';
import type { Menu } from './menu.types';
import { create, getDetail, getList, getTree, update } from './menu.api';
import { menuQueryKeys } from './menu.types';

export {
    batchCreateButtons,
    create,
    getDetail,
    getList,
    getTree,
    remove,
    reorder,
    update,
} from './menu.api';

type MenuListResult = PaginatedResult<Menu.Item>;

export function useMenuList(
    params?: Menu.Query,
    options?: Omit<UseQueryOptions<MenuListResult>, 'queryKey' | 'queryFn'>
) {
    return useQuery({
        queryKey: menuQueryKeys.list(params),
        queryFn: () => getList(params),
        ...options,
    });
}

export function useMenuTree(
    status?: Menu.Status,
    options?: Omit<UseQueryOptions<Menu.TreeItem[]>, 'queryKey' | 'queryFn'>
) {
    return useQuery({
        queryKey: menuQueryKeys.tree(status),
        queryFn: () => getTree(status),
        staleTime: 5 * 60 * 1000,
        ...options,
    });
}

export function useMenuDetail(
    id: number,
    options?: Omit<UseQueryOptions<Menu.Item>, 'queryKey' | 'queryFn'>
) {
    return useQuery({
        queryKey: menuQueryKeys.detail(id),
        queryFn: () => getDetail(id),
        enabled: id > 0,
        ...options,
    });
}

export function useMenuTreeSelect(status?: Menu.Status) {
    const { data: tree = [], ...rest } = useMenuTree(status);

    const treeData = useMemo((): TreeSelectProps['treeData'] => {
        const loop = (nodes: Menu.TreeItem[]): TreeSelectProps['treeData'] =>
            nodes.map((n) => ({
                title: n.name,
                value: n.id,
                children: n.children?.length ? loop(n.children) : undefined,
            }));
        return loop(tree);
    }, [tree]);

    return {
        ...rest,
        tree,
        treeData,
    };
}

interface MenuTreeNode {
    title: string;
    key: number;
    children?: MenuTreeNode[];
}

export function useMenuTreeForPermission() {
    const { data: tree = [], ...rest } = useMenuTree('enabled');

    const treeData = useMemo(() => {
        const loop = (nodes: Menu.TreeItem[]): MenuTreeNode[] =>
            nodes.map((n) => ({
                title: n.name,
                key: n.id,
                children: n.children?.length ? loop(n.children) : undefined,
            }));
        return loop(tree);
    }, [tree]);

    const allMenuIds = useMemo(() => {
        const ids: number[] = [];
        const collect = (nodes: Menu.TreeItem[]) => {
            nodes.forEach((n) => {
                ids.push(n.id);
                if (n.children?.length) collect(n.children);
            });
        };
        collect(tree);
        return ids;
    }, [tree]);

    return {
        ...rest,
        tree,
        treeData,
        allMenuIds,
    };
}

export function useMenuSave() {
    return useSaveMutation<Menu.CreateDto & { id?: number }, Menu.CreateDto, Menu.UpdateDto>({
        createFn: create,
        updateFn: update,
        toUpdatePayload: ({ id: _id, ...data }) => data,
        createMessage: '保存成功',
        updateMessage: '保存成功',
        invalidateKeys: [menuQueryKeys.all],
    });
}
