import { useMemo } from 'react';
import type { NavigationItem } from '@/config/navigation.types';
import { useAuthStore } from '@/store/authStore';

interface DynamicRoutesResult {
    pageMenus: NavigationItem[];
    defaultPath: string;
    isLoading: boolean;
}

/**
 * 动态路由 Hook
 * 从用户菜单中提取页面路由信息,并缓存结果
 */
export function useDynamicRoutes(): DynamicRoutesResult {
    const user = useAuthStore((s) => s.user);
    const token = useAuthStore((s) => s.token);

    const { pageMenus, defaultPath } = useMemo(() => {
        const menus: NavigationItem[] = user?.menus ?? [];

        const pages = menus.filter(
            (m) => m.type === 'page' && m.component && m.path && m.path.trim().length > 0
        );

        const defaultRoute = pages[0]?.path || '/dashboard';

        return {
            pageMenus: pages,
            defaultPath: defaultRoute,
        };
    }, [user?.menus]);

    const isLoading = !!token && !user;

    return {
        pageMenus,
        defaultPath,
        isLoading,
    };
}
