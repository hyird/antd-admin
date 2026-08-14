import type { ComponentType } from 'react';
import type { AppIconName } from '@/utils/icon';

export interface PermissionConfig {
    code: string;
    name: string;
    description?: string;
    module: string;
    resource: string;
    action: 'query' | 'add' | 'edit' | 'delete' | 'perm' | 'export' | 'import' | string;
}

export interface PageConfig {
    component: string;
    name: string;
    icon?: AppIconName;
    description?: string;
    module: string;
    loader: () => Promise<{ default: ComponentType<unknown> }>;
    permissions?: readonly Omit<PermissionConfig, 'module' | 'resource'>[];
}
