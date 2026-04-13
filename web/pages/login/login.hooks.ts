/**
 * 认证相关 Hooks
 */

import { useMutation } from '@tanstack/react-query';
import { App } from 'antd';
import { useNavigate } from 'react-router-dom';
import { useAuthStore } from '@/store/authStore';
import type { Auth } from './login.types';
import { login, logout } from './login.api';

/**
 * 登录 Hook
 */
export function useLogin() {
    const navigate = useNavigate();
    const { message } = App.useApp();
    const setAuth = useAuthStore((s) => s.setAuth);

    return useMutation({
        mutationFn: login,
        onSuccess: (data: Auth.LoginResult) => {
            setAuth(data.token, data.refreshToken, data.user);
            message.success('登录成功');
            navigate('/', { replace: true });
        },
    });
}

/**
 * 登出 Hook
 */
export function useLogout() {
    const navigate = useNavigate();
    const refreshToken = useAuthStore((s) => s.refreshToken);
    const clearAuth = useAuthStore((s) => s.clearAuth);

    return useMutation({
        mutationFn: () => logout(refreshToken ?? undefined),
        onSettled: () => {
            clearAuth();
            navigate('/login', { replace: true });
        },
    });
}
