/**
 * 认证相关 Queries
 */

import { useQuery } from '@tanstack/react-query';
import { useAuthStore } from '@/store/authStore';
import { deepEqual } from '@/utils/tree';
import { fetchCurrentUser } from './login.api';

export const loginKeys = {
    currentUser: ['auth', 'currentUser'] as const,
};

/**
 * 获取当前用户信息
 * - 使用 zustand 持久化数据作为 initialData（页面刷新时立即可用）
 * - 自动后台刷新 + 定时轮询（替代 useHeartbeat）
 * - 查询结果同步到 zustand（用于菜单持久化和路由渲染）
 */
export function useCurrentUser() {
    const token = useAuthStore((s) => s.token);
    const user = useAuthStore((s) => s.user);
    const setUser = useAuthStore((s) => s.setUser);

    return useQuery({
        queryKey: loginKeys.currentUser,
        queryFn: async () => {
            const freshUser = await fetchCurrentUser();
            if (!deepEqual(user, freshUser)) {
                setUser(freshUser);
            }
            return freshUser;
        },
        enabled: !!token,
        initialData: user ?? undefined,
        initialDataUpdatedAt: 0,
        staleTime: 2 * 60 * 1000,
        refetchInterval: 5 * 60 * 1000,
        refetchOnWindowFocus: true,
    });
}
