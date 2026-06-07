/**
 * 页面注册入口
 * 集中注册所有页面，保持懒加载
 */

import type { ComponentType } from 'react';
import type { AppIconName } from '@/utils/icon';

export interface PageConfig {
    component: string;
    name: string;
    icon?: AppIconName;
    description?: string;
    module: string;
    loader: () => Promise<{ default: ComponentType<unknown> }>;
    permissions?: Omit<PermissionConfig, 'module' | 'resource'>[];
}

export interface PermissionConfig {
    code: string;
    name: string;
    description?: string;
    module: string;
    resource: string;
    action: 'query' | 'add' | 'edit' | 'delete' | 'perm' | 'export' | 'import' | string;
}

export interface ModuleConfig {
    name: string;
    pages: Omit<PageConfig, 'module'>[];
}

const pageRegistry: PageConfig[] = [];
const permissionRegistry: PermissionConfig[] = [];

export function registerPage(config: PageConfig): void {
    const exists = pageRegistry.find((p) => p.component === config.component);
    if (exists) {
        return;
    }
    pageRegistry.push(config);

    if (config.permissions) {
        config.permissions.forEach((permission) => {
            registerPermission({
                ...permission,
                module: config.module,
                resource: config.name,
            });
        });
    }
}

export function registerPages(...configs: PageConfig[]): void {
    configs.forEach(registerPage);
}

export function getRegisteredPages(): PageConfig[] {
    return [...pageRegistry];
}

export function getPagesByModule(module: string): PageConfig[] {
    return pageRegistry.filter((p) => p.module === module);
}

export function getPageConfig(component: string): PageConfig | undefined {
    return pageRegistry.find((p) => p.component === component);
}

export function getPageIcon(component?: string): string | undefined {
    if (!component) {
        return undefined;
    }

    return getPageConfig(component)?.icon;
}

export function getComponentLoaderMap(): Record<
    string,
    () => Promise<{ default: ComponentType<unknown> }>
> {
    return Object.fromEntries(pageRegistry.map((page) => [page.component, page.loader]));
}

export function registerPermission(config: PermissionConfig): void {
    const exists = permissionRegistry.find((p) => p.code === config.code);
    if (exists) {
        return;
    }
    permissionRegistry.push(config);
}

export function registerPermissions(...configs: PermissionConfig[]): void {
    configs.forEach(registerPermission);
}

export function getRegisteredPermissions(): PermissionConfig[] {
    return [...permissionRegistry];
}

export function getPermissionsByModule(module: string): PermissionConfig[] {
    return permissionRegistry.filter((p) => p.module === module);
}

export function getPermissionsByResource(resource: string): PermissionConfig[] {
    return permissionRegistry.filter((p) => p.resource === resource);
}

export function getPermissionConfig(code: string): PermissionConfig | undefined {
    return permissionRegistry.find((p) => p.code === code);
}

export function registerModule(config: ModuleConfig): void {
    config.pages.forEach((page) => {
        registerPage({ ...page, module: config.name });

        if (page.permissions) {
            page.permissions.forEach((permission) => {
                registerPermission({
                    ...permission,
                    module: config.name,
                    resource: page.name,
                });
            });
        }
    });
}

// 首页
registerPage({
    component: 'Home',
    name: '首页',
    icon: 'HomeOutlined',
    module: '首页',
    description: '系统概览和数据统计',
    loader: () => import('./home/index'),
    permissions: [
        {
            code: 'home:dashboard:query',
            name: '查看统计',
            description: '查看首页统计数据和系统信息',
            action: 'query',
        },
    ],
});

// 用户管理
registerPage({
    component: 'User',
    name: '用户管理',
    icon: 'UserOutlined',
    module: '系统管理',
    description: '系统用户的增删改查、角色分配',
    loader: () => import('./system/user'),
    permissions: [
        { code: 'system:user:query', name: '查询用户', action: 'query' },
        { code: 'system:user:add', name: '新增用户', action: 'add' },
        { code: 'system:user:edit', name: '编辑用户', action: 'edit' },
        { code: 'system:user:delete', name: '删除用户', action: 'delete' },
    ],
});

// 角色管理
registerPage({
    component: 'Role',
    name: '角色管理',
    icon: 'SafetyCertificateOutlined',
    module: '系统管理',
    description: '角色的增删改查、权限分配',
    loader: () => import('./system/role'),
    permissions: [
        { code: 'system:role:query', name: '查询角色', action: 'query' },
        { code: 'system:role:add', name: '新增角色', action: 'add' },
        { code: 'system:role:edit', name: '编辑角色', action: 'edit' },
        { code: 'system:role:delete', name: '删除角色', action: 'delete' },
        { code: 'system:role:perm', name: '分配权限', action: 'perm' },
    ],
});

// 部门管理
registerPage({
    component: 'Dept',
    name: '部门管理',
    icon: 'ApartmentOutlined',
    module: '系统管理',
    description: '组织架构的树形管理',
    loader: () => import('./system/dept'),
    permissions: [
        { code: 'system:dept:query', name: '查询部门', action: 'query' },
        { code: 'system:dept:add', name: '新增部门', action: 'add' },
        { code: 'system:dept:edit', name: '编辑部门', action: 'edit' },
        { code: 'system:dept:delete', name: '删除部门', action: 'delete' },
    ],
});

// 菜单管理
registerPage({
    component: 'Menu',
    name: '菜单管理',
    icon: 'MenuOutlined',
    module: '系统管理',
    description: '菜单、页面与按钮权限配置',
    loader: () => import('./system/menu'),
    permissions: [
        { code: 'system:menu:query', name: '查询菜单', action: 'query' },
        { code: 'system:menu:add', name: '新增菜单', action: 'add' },
        { code: 'system:menu:edit', name: '编辑菜单', action: 'edit' },
        { code: 'system:menu:delete', name: '删除菜单', action: 'delete' },
    ],
});
