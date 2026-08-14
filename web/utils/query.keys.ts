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
