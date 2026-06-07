import { useCurrentUser } from '@/pages/login';
import { useAuthStore } from '@/store/authStore';

/**
 * 初始化用户认证数据
 * 使用 TanStack Query 管理用户数据获取和刷新
 */
export function useInitAuth() {
    const token = useAuthStore((s) => s.token);
    const { data: user } = useCurrentUser();

    return { token, user };
}
