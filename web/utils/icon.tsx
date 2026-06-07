import type { ComponentType, CSSProperties, ReactNode } from 'react';
import * as AntIcons from '@ant-design/icons';
import DynamicIcon from '@/components/DynamicIcon';
import type { Menu } from '@/pages/system/menu/menu.types';

type IconComp = ComponentType<{ style?: CSSProperties; className?: string }>;

function isAntdIconName(name: string): boolean {
    return name.endsWith('Outlined') || name.endsWith('Filled') || name.endsWith('TwoTone');
}

export const appIconMap: Record<string, IconComp> = Object.entries(AntIcons)
    .filter(([name]) => isAntdIconName(name))
    .sort(([a], [b]) => a.localeCompare(b))
    .reduce<Record<string, IconComp>>((acc, [name, icon]) => {
        acc[name] = icon as IconComp;
        return acc;
    }, {});

export type AppIconName = keyof typeof appIconMap;

export function isAppIconName(name?: string): name is AppIconName {
    if (!name) {
        return false;
    }

    return name in appIconMap;
}

export function resolveMenuIconName(
    menu: Pick<Menu.Item, 'type' | 'component' | 'icon'>
): string | undefined {
    return menu.icon;
}

export function renderIcon(iconName?: string): ReactNode {
    if (!iconName) return undefined;
    return <DynamicIcon name={iconName} />;
}

export function renderMenuIcon(menu: Pick<Menu.Item, 'type' | 'component' | 'icon'>): ReactNode {
    return renderIcon(resolveMenuIconName(menu));
}
