import { create, type StateCreator } from 'zustand';
import { persist } from 'zustand/middleware';
import type { Auth } from '@/pages/login/login.types';
import { refreshToken } from '@/pages/login/login.api';

interface AuthState {
    token: string | null;
    refreshToken: string | null;
    user: Auth.UserInfo | null;
}

interface AuthActions {
    setAuth: (token: string, refreshToken: string, user: Auth.UserInfo) => void;
    clearAuth: () => void;
    setUser: (user: Auth.UserInfo) => void;
    refreshAccessToken: () => Promise<boolean>;
}

export type AuthStore = AuthState & AuthActions;

const authStateCreator: StateCreator<AuthStore> = (set, get) => ({
    token: null,
    refreshToken: null,
    user: null,

    setAuth: (token, refreshToken, user) => {
        set({ token, refreshToken, user });
    },

    clearAuth: () => {
        set({ token: null, refreshToken: null, user: null });
    },

    setUser: (user) => {
        set({ user });
    },

    refreshAccessToken: async () => {
        const { refreshToken: currentRefreshToken } = get();
        if (!currentRefreshToken) return false;

        try {
            const { token, refreshToken: nextRefreshToken, user } = await refreshToken(
                currentRefreshToken,
                {
                _silent: true,
                }
            );
            set({ token, refreshToken: nextRefreshToken, user });
            return true;
        } catch {
            set({ token: null, refreshToken: null, user: null });
            return false;
        }
    },
});

export const useAuthStore = create<AuthStore>()(
    persist(authStateCreator, {
        name: 'auth-storage',
        storage: {
            getItem: (key) => {
                const value = sessionStorage.getItem(key);
                return value ? JSON.parse(value) : null;
            },
            setItem: (key, value) => {
                sessionStorage.setItem(key, JSON.stringify(value));
            },
            removeItem: (key) => {
                sessionStorage.removeItem(key);
            },
        },
        partialize: (state) =>
            ({
                token: state.token,
                refreshToken: state.refreshToken,
                user: state.user,
            }) as AuthStore,
    })
);
