import { repo } from '@/config/data';

interface UserPermissionCache {
    permissions: Set<string>;
    isSuperadmin: boolean;
    expireAt: number;
}

const CACHE_TTL = 60 * 1000;
const permissionCache = new Map<number, UserPermissionCache>();

export class PermissionService {
    async getUserPermissions(
        userId: number
    ): Promise<{ permissions: Set<string>; isSuperadmin: boolean }> {
        const now = Date.now();
        const cached = permissionCache.get(userId);
        if (cached && cached.expireAt > now) {
            return {
                permissions: cached.permissions,
                isSuperadmin: cached.isSuperadmin,
            };
        }

        const user = await repo.user.findOne({
            where: { id: userId },
            relations: ['roles', 'roles.menus'],
        });

        if (!user) {
            return { permissions: new Set(), isSuperadmin: false };
        }

        const isSuperadmin =
            user.roles?.some((r) => r.code === 'superadmin' && r.status === 'enabled') ?? false;

        const permissions = new Set<string>();
        user.roles?.forEach((role) => {
            if (role.status === 'enabled') {
                role.menus?.forEach((menu) => {
                    if (menu.status === 'enabled' && menu.permission_code) {
                        permissions.add(menu.permission_code);
                    }
                });
            }
        });

        permissionCache.set(userId, {
            permissions,
            isSuperadmin,
            expireAt: now + CACHE_TTL,
        });

        return { permissions, isSuperadmin };
    }

    clearUserCache(userId: number): void {
        permissionCache.delete(userId);
    }

    clearAllCache(): void {
        permissionCache.clear();
    }

    async hasPermission(userId: number, code: string): Promise<boolean> {
        const { permissions, isSuperadmin } = await this.getUserPermissions(userId);
        return isSuperadmin || permissions.has(code);
    }

    async hasAnyPermission(userId: number, codes: string[]): Promise<boolean> {
        const { permissions, isSuperadmin } = await this.getUserPermissions(userId);
        return isSuperadmin || codes.some((code) => permissions.has(code));
    }

    async hasAllPermissions(userId: number, codes: string[]): Promise<boolean> {
        const { permissions, isSuperadmin } = await this.getUserPermissions(userId);
        return isSuperadmin || codes.every((code) => permissions.has(code));
    }
}

export const permissionService = new PermissionService();
