import type { CSSProperties } from 'react';
import { appIconMap, isAppIconName } from '@/utils/icon';

interface DynamicIconProps {
    name?: string;
    style?: CSSProperties;
    className?: string;
}

export default function DynamicIcon({ name, ...props }: DynamicIconProps) {
    const Icon = isAppIconName(name) ? appIconMap[name] : undefined;

    if (!Icon) return null;
    return <Icon {...props} />;
}
