/**
 * React Query 工具
 */

import { QueryClient } from '@tanstack/react-query';

export const queryClient = new QueryClient({
    defaultOptions: {
        queries: {
            staleTime: 5 * 60 * 1000,
            gcTime: 10 * 60 * 1000,
            refetchOnWindowFocus: false,
            refetchOnReconnect: true,
            retry: 1,
        },
        mutations: {
            retry: 0,
        },
    },
});

export const deviceKeys = {
    all: ['device'] as const,
    realtime: () => ['device', 'realtime'] as const,
};

export function createQueryKeys<T extends string>(module: T) {
    return {
        all: [module] as const,
        lists: () => [module, 'list'] as const,
        list: <P extends Record<string, unknown>>(params: P) => [module, 'list', params] as const,
        details: () => [module, 'detail'] as const,
        detail: (id: number | string) => [module, 'detail', id] as const,
        trees: () => [module, 'tree'] as const,
        tree: <P extends Record<string, unknown>>(params?: P) => [module, 'tree', params] as const,
        options: () => [module, 'options'] as const,
    };
}
