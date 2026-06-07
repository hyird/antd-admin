import { useMemo } from 'react';
import { useCurrentUser } from '@/pages/login';
import { SUPERADMIN_ROLE_CODE } from '@/config/app';

function usePermissionState() {
    const { data: user } = useCurrentUser();

    return useMemo(() => {
        if (!user) return { isSuperAdmin: false, codesSet: new Set<string>() };

        const superAdmin = user.roles?.some((r) => r.code === SUPERADMIN_ROLE_CODE) ?? false;
        const codes = new Set<string>();
        user.menus?.forEach((m) => {
            if (m.type === 'button' && m.permission_code) {
                codes.add(m.permission_code);
            }
        });
        return { isSuperAdmin: superAdmin, codesSet: codes };
    }, [user]);
}

export function usePermissions() {
    const { isSuperAdmin, codesSet } = usePermissionState();

    const has = useMemo(
        () =>
            (code?: string): boolean => {
                if (!code) return true;
                if (isSuperAdmin) return true;
                return codesSet.has(code);
            },
        [isSuperAdmin, codesSet]
    );

    return {
        isSuperAdmin,
        has,
        codesSet,
    };
}

export function usePermission(code?: string): boolean {
    const { has } = usePermissions();
    return has(code);
}
