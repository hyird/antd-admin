/**
 * 通用树构建工具
 */

import type { NavigationItem, NavigationTreeItem } from '@/config/navigation.types';

/** 树节点基础接口 */
export interface TreeNode {
    id: number;
    parent_id?: number | null;
    sort_order?: number;
    children?: TreeNode[];
}

/** 构建树的选项 */
export interface BuildTreeOptions {
    /** 排序字段 */
    sortBy?: 'order' | 'id' | 'code' | 'name';
    /** 是否移除空的 children 数组 */
    removeEmptyChildren?: boolean;
    /** 自定义比较函数 */
    compareFn?: <T>(a: T, b: T) => number;
}

/**
 * 将平铺列表构建成树形结构
 * @param items 平铺的数据列表
 * @param options 配置选项
 */
export function buildTree<T extends TreeNode>(
    items: T[],
    options: BuildTreeOptions = {}
): (T & { children?: T[] })[] {
    const { sortBy = 'order', removeEmptyChildren = true, compareFn } = options;

    const map = new Map<number, T & { children: T[] }>();
    const roots: (T & { children: T[] })[] = [];

    // 创建所有节点的映射
    items.forEach((item) => {
        map.set(item.id, { ...item, children: [] });
    });

    const hasParentCycle = (item: T): boolean => {
        const seen = new Set<number>([item.id]);
        let parentId = item.parent_id;

        while (parentId && map.has(parentId)) {
            if (seen.has(parentId)) return true;
            seen.add(parentId);
            parentId = map.get(parentId)?.parent_id;
        }

        return false;
    };

    // 构建父子关系
    items.forEach((item) => {
        // biome-ignore lint/style/noNonNullAssertion: map.get is safe here as item was just inserted above
        const node = map.get(item.id)!;
        const parentId = item.parent_id;
        if (parentId && map.has(parentId) && !hasParentCycle(item)) {
            map.get(parentId)?.children.push(node);
        } else {
            roots.push(node);
        }
    });

    // 排序函数
    const defaultCompareFn = (a: T, b: T): number => {
        const aRecord = a as unknown as Record<string, unknown>;
        const bRecord = b as unknown as Record<string, unknown>;
        const getSortValue = (record: Record<string, unknown>) =>
            (record.sort_order as number) ?? 0;
        if (sortBy === 'order') {
            return getSortValue(aRecord) - getSortValue(bRecord);
        }
        if (sortBy === 'id') {
            return a.id - b.id;
        }
        if (sortBy === 'code') {
            return ((aRecord.code as string) ?? '').localeCompare((bRecord.code as string) ?? '');
        }
        if (sortBy === 'name') {
            return ((aRecord.name as string) ?? '').localeCompare((bRecord.name as string) ?? '');
        }
        return 0;
    };

    const sortFn = compareFn ?? defaultCompareFn;

    // 递归排序并清理空 children
    const sortTree = (
        nodes: (T & { children: T[] })[],
        path = new Set<number>()
    ): (T & { children?: T[] })[] => {
        return nodes.sort(sortFn).map((n) => {
            const result: T & { children?: T[] } = { ...n };
            const nextPath = new Set(path);
            nextPath.add(n.id);
            const children = n.children.filter((child) => !nextPath.has(child.id));
            if (children.length > 0) {
                result.children = sortTree(children as (T & { children: T[] })[], nextPath);
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
 * @param tree 树形数据
 */
export function flattenTree<T extends TreeNode>(tree: T[]): T[] {
    const result: T[] = [];

    const traverse = (nodes: T[], path = new Set<number>()) => {
        nodes.forEach((node) => {
            if (path.has(node.id)) return;
            const nextPath = new Set(path);
            nextPath.add(node.id);
            const { children, ...rest } = node as T & { children?: TreeNode[] };
            result.push(rest as T);
            if (children && children.length > 0) {
                traverse(children as T[], nextPath);
            }
        });
    };

    traverse(tree);
    return result;
}

/**
 * 将树形结构转为 id -> node 的映射
 * @param tree 树形数据
 */
export function treeToMap<T extends TreeNode>(tree: T[]): Map<number, T> {
    const map = new Map<number, T>();

    const traverse = (nodes: T[], path = new Set<number>()) => {
        nodes.forEach((node) => {
            if (path.has(node.id)) return;
            const nextPath = new Set(path);
            nextPath.add(node.id);
            map.set(node.id, node);
            if (node.children && node.children.length > 0) {
                traverse(node.children as T[], nextPath);
            }
        });
    };

    traverse(tree);
    return map;
}

/**
 * 按关键词过滤树（保留匹配节点及其祖先）
 * @param tree 树形数据
 * @param keyword 关键词
 * @param matchFields 要匹配的字段名数组
 */
export function filterTree<T extends TreeNode>(
    tree: T[],
    keyword: string,
    matchFields: (keyof T)[] = ['name' as keyof T]
): T[] {
    const kw = keyword.trim().toLowerCase();
    if (!kw) return tree;

    const filter = (nodes: T[], path = new Set<number>()): T[] => {
        const result: T[] = [];

        nodes.forEach((node) => {
            if (path.has(node.id)) return;
            const nextPath = new Set(path);
            nextPath.add(node.id);

            // 检查当前节点是否匹配
            const selfMatch = matchFields.some((field) => {
                const value = node[field];
                return typeof value === 'string' && value.toLowerCase().includes(kw);
            });

            // 递归过滤子节点
            const children = node.children ? filter(node.children as T[], nextPath) : [];

            // 如果自身匹配或有匹配的子节点，保留该节点
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

/**
 * 查找树中的节点
 * @param tree 树形数据
 * @param predicate 匹配函数
 */
export function findInTree<T extends TreeNode>(
    tree: T[],
    predicate: (node: T) => boolean
): T | undefined {
    const find = (nodes: T[], path = new Set<number>()): T | undefined => {
        for (const node of nodes) {
            if (path.has(node.id)) continue;
            const nextPath = new Set(path);
            nextPath.add(node.id);

            if (predicate(node)) {
                return node;
            }
            if (node.children && node.children.length > 0) {
                const found = find(node.children as T[], nextPath);
                if (found) return found;
            }
        }
        return undefined;
    };

    return find(tree);
}

/**
 * 获取节点的所有子孙 ID
 * @param tree 树形数据
 * @param nodeId 节点 ID
 */
export function getDescendantIds<T extends TreeNode>(tree: T[], nodeId: number): number[] {
    const ids: number[] = [];

    const collectIds = (nodes: T[], path = new Set<number>()) => {
        nodes.forEach((node) => {
            if (path.has(node.id)) return;
            const nextPath = new Set(path);
            nextPath.add(node.id);
            ids.push(node.id);
            if (node.children && node.children.length > 0) {
                collectIds(node.children as T[], nextPath);
            }
        });
    };

    const node = findInTree(tree, (n) => n.id === nodeId);
    if (node?.children) {
        collectIds(node.children as T[], new Set([node.id]));
    }

    return ids;
}

/**
 * 获取节点的祖先路径（从根到父节点）
 * @param tree 树形数据
 * @param nodeId 节点 ID
 */
export function getAncestorPath<T extends TreeNode>(tree: T[], nodeId: number): T[] {
    const path: T[] = [];

    const find = (nodes: T[], ancestors: T[], visited = new Set<number>()): boolean => {
        for (const node of nodes) {
            if (visited.has(node.id)) continue;
            const nextVisited = new Set(visited);
            nextVisited.add(node.id);

            if (node.id === nodeId) {
                path.push(...ancestors);
                return true;
            }
            if (node.children && node.children.length > 0) {
                if (find(node.children as T[], [...ancestors, node], nextVisited)) {
                    return true;
                }
            }
        }
        return false;
    };

    find(tree, []);
    return path;
}

// ============ 菜单专用函数 ============

const filteredMenuTreeCache = new WeakMap<NavigationItem[], NavigationTreeItem[]>();
const fullMenuTreeCache = new WeakMap<NavigationItem[], NavigationTreeItem[]>();

/**
 * 将平铺的菜单列表构建成树形结构
 * @param menus 菜单列表
 * @param filterButton 是否过滤掉 button 类型，默认 true
 */
export function buildMenuTree(menus: NavigationItem[], filterButton = true): NavigationTreeItem[] {
    const cache = filterButton ? filteredMenuTreeCache : fullMenuTreeCache;
    const cachedTree = cache.get(menus);
    if (cachedTree) {
        return cachedTree;
    }

    const items = filterButton ? menus.filter((m) => m.type !== 'button') : menus;
    const tree = buildTree(items, { sortBy: 'order' }) as NavigationTreeItem[];

    // 计算完整路径
    const computeFullPath = (
        nodes: NavigationTreeItem[],
        parentPath = '',
        path = new Set<number>()
    ) => {
        for (const node of nodes) {
            if (path.has(node.id)) continue;
            const nextPath = new Set(path);
            nextPath.add(node.id);

            if (node.path) {
                // 如果路径以 / 开头，使用绝对路径；否则拼接父路径
                const fullPath = node.path.startsWith('/')
                    ? node.path
                    : `${parentPath}/${node.path}`;
                node.full_path = fullPath;
            } else {
                node.full_path = parentPath;
            }
            if (node.children) {
                computeFullPath(node.children, node.full_path, nextPath);
            }
        }
    };

    computeFullPath(tree);
    cache.set(menus, tree);
    return tree;
}

/**
 * 按关键词过滤菜单树
 * @param tree 菜单树
 * @param keyword 关键词
 */
export function filterMenuTree(tree: NavigationTreeItem[], keyword: string): NavigationTreeItem[] {
    return filterTree(tree, keyword, ['name', 'path']) as NavigationTreeItem[];
}

/**
 * 获取路径段（相对于父级的路径）
 * @param record 当前菜单项
 * @param menuMap 菜单映射表
 */
export function getPathSegment(
    record: NavigationTreeItem,
    menuMap: Map<number, NavigationTreeItem> | Record<number, NavigationTreeItem>
): string {
    const fullPath = (record.full_path || record.path || '').trim();
    if (!fullPath) return '';
    const parentId = record.parent_id;
    if (!parentId) {
        return fullPath.replace(/^\/+/, '');
    }
    const parent = menuMap instanceof Map ? menuMap.get(parentId) : menuMap[parentId];
    const parentPath = (parent?.full_path || parent?.path || '').trim();
    if (!parentPath) {
        return fullPath.replace(/^\/+/, '');
    }
    if (fullPath.startsWith(parentPath)) {
        const seg = fullPath.slice(parentPath.length);
        return seg.replace(/^\/+/, '');
    }
    return fullPath.replace(/^\/+/, '');
}
