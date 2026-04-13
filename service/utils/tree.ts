/**
 * 通用树构建工具
 */

import type { MenuItem, MenuTreeItem } from '../modules/system/menu/menu.types';

/**
 * 树节点基础接口
 */
export interface TreeNode {
    id: number;
    parentId?: number | null;
    parent_id?: number | null;
    order?: number;
    sort_order?: number;
    children?: TreeNode[];
}

/**
 * 树构建配置
 */
export interface BuildTreeOptions {
    /** 排序字段，默认 'order' */
    sortBy?: 'order' | 'id' | 'code' | 'name';
    /** 是否移除空的 children 数组，默认 true */
    removeEmptyChildren?: boolean;
    /** 自定义排序比较函数 */
    compareFn?: (a: TreeNode, b: TreeNode) => number;
}

/**
 * 将平铺列表构建成树形结构
 */
export function buildTree<T extends TreeNode>(
    items: T[],
    options: BuildTreeOptions = {}
): (T & { children?: T[] })[] {
    const { sortBy = 'order', removeEmptyChildren = true, compareFn } = options;

    const map = new Map<number, T & { children: T[] }>();
    const roots: (T & { children: T[] })[] = [];

    items.forEach((item) => {
        map.set(item.id, { ...item, children: [] });
    });

    items.forEach((item) => {
        const node = map.get(item.id)!;
        const parentId = item.parentId ?? item.parent_id;
        if (parentId && map.has(parentId)) {
            map.get(parentId)!.children.push(node);
        } else {
            roots.push(node);
        }
    });

    const defaultCompareFn = (a: T, b: T): number => {
        const getOrder = (value: T): number => {
            const record = value as unknown as Record<string, unknown>;
            return (record.order as number) ?? (record.sort_order as number) ?? 0;
        };
        const getText = (value: T, key: 'code' | 'name'): string => {
            const record = value as unknown as Record<string, unknown>;
            return String(record[key] ?? '');
        };
        if (sortBy === 'order') {
            return getOrder(a) - getOrder(b);
        }
        if (sortBy === 'id') {
            return a.id - b.id;
        }
        if (sortBy === 'code') {
            return getText(a, 'code').localeCompare(getText(b, 'code'));
        }
        if (sortBy === 'name') {
            return getText(a, 'name').localeCompare(getText(b, 'name'));
        }
        return 0;
    };

    const sortFn = compareFn ?? defaultCompareFn;

    const sortTree = (nodes: (T & { children: T[] })[]): (T & { children?: T[] })[] => {
        return nodes.sort(sortFn).map((n) => {
            const result: T & { children?: T[] } = { ...n };
            if (n.children.length > 0) {
                result.children = sortTree(n.children as (T & { children: T[] })[]);
            } else if (removeEmptyChildren) {
                delete result.children;
            } else {
                result.children = [];
            }
            return result;
        });
    };

    return sortTree(roots);
}

/**
 * 将树形结构平铺为列表
 */
export function flattenTree<T extends TreeNode>(tree: T[]): T[] {
    const result: T[] = [];

    const traverse = (nodes: T[]) => {
        nodes.forEach((node) => {
            const { children, ...rest } = node as T & { children?: T[] };
            result.push(rest as T);
            if (children && children.length > 0) {
                traverse(children);
            }
        });
    };

    traverse(tree);
    return result;
}

/**
 * 按关键词过滤树（保留匹配节点及其祖先）
 */
export function filterTree<T extends TreeNode>(
    tree: T[],
    keyword: string,
    matchFields: (keyof T)[] = ['name' as keyof T]
): T[] {
    const kw = keyword.trim().toLowerCase();
    if (!kw) return tree;

    const filter = (nodes: T[]): T[] => {
        const result: T[] = [];

        nodes.forEach((node) => {
            const selfMatch = matchFields.some((field) => {
                const value = node[field];
                return typeof value === 'string' && value.toLowerCase().includes(kw);
            });

            const children = node.children ? filter(node.children as T[]) : [];

            if (selfMatch || children.length > 0) {
                const newNode = { ...node } as T & { children?: T[] };
                if (children.length > 0) {
                    newNode.children = children;
                } else {
                    delete newNode.children;
                }
                result.push(newNode as T);
            }
        });

        return result;
    };

    return filter(tree);
}

// ============ 菜单树特定工具 ============

/**
 * 将平铺的菜单列表构建成树形结构
 */
export function buildMenuTree(menus: MenuItem[], filterButton = true): MenuTreeItem[] {
    const menuItems = filterButton ? menus.filter((m) => m.type !== 'button') : menus;
    return buildTree(menuItems, { sortBy: 'order' }) as MenuTreeItem[];
}

/**
 * 按关键词过滤菜单树
 */
export function filterMenuTree(tree: MenuTreeItem[], keyword: string): MenuTreeItem[] {
    return filterTree(tree, keyword, ['name', 'path']) as MenuTreeItem[];
}
