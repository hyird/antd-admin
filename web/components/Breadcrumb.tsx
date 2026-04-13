import { DownOutlined, HomeOutlined } from '@ant-design/icons';
import type { MenuProps } from 'antd';
import { Breadcrumb, Dropdown } from 'antd';
import { useMemo } from 'react';
import { useLocation, useNavigate } from 'react-router-dom';
import DynamicIcon from '@/components/DynamicIcon';
import { useAuthStore } from '@/store/authStore';
import type { Menu } from '@/pages/system/menu/menu.types';
import { buildMenuTree } from '@/utils/tree';
import { resolveMenuIconName } from '@/utils/icon';

interface BreadcrumbItemData {
    item: Menu.TreeItem;
    isLast: boolean;
}

export default function AppBreadcrumb() {
    const location = useLocation();
    const navigate = useNavigate();
    const { user } = useAuthStore();

    const menuTree = useMemo<Menu.TreeItem[]>(() => {
        const menus = user?.menus || [];
        return buildMenuTree(menus);
    }, [user?.menus]);

    const pathMap = useMemo(() => {
        const map = new Map<string, Menu.TreeItem[]>();

        const traverse = (items: Menu.TreeItem[], parents: Menu.TreeItem[] = []) => {
            for (const item of items) {
                if (item.full_path) {
                    map.set(item.full_path, [...parents, item]);
                }
                if (item.children) {
                    traverse(item.children, [...parents, item]);
                }
            }
        };

        traverse(menuTree);
        return map;
    }, [menuTree]);

    const breadcrumbData = useMemo<BreadcrumbItemData[]>(() => {
        const chain = pathMap.get(location.pathname);
        if (!chain || chain.length === 0) {
            return [];
        }

        return chain.map((item, index) => ({
            item,
            isLast: index === chain.length - 1,
        }));
    }, [location.pathname, pathMap]);

    // 首页显示默认面包屑
    if (location.pathname === '/home') {
        return (
            <Breadcrumb
                items={[
                    {
                        title: (
                            <span className="inline-flex items-center gap-1">
                                <HomeOutlined />
                                首页
                            </span>
                        ),
                    },
                ]}
            />
        );
    }

    if (breadcrumbData.length === 0) {
        return null;
    }

    const renderBreadcrumbItem = (data: BreadcrumbItemData) => {
        const { item, isLast } = data;
        const iconName = resolveMenuIconName(item);

        const pageChildren =
            item.children?.filter((child) => child.type === 'page' && child.full_path) || [];

        if (pageChildren.length > 0) {
            const menuItems: MenuProps['items'] = pageChildren.map((child) => ({
                key: child.id,
                icon: resolveMenuIconName(child) ? (
                    <DynamicIcon name={resolveMenuIconName(child)} />
                ) : null,
                label: child.name,
                disabled: location.pathname === child.full_path,
                onClick: () => {
                    if (child.full_path && location.pathname !== child.full_path) {
                        navigate(child.full_path);
                    }
                },
            }));

            return (
                <Dropdown menu={{ items: menuItems }} trigger={['hover']}>
                    <span className="inline-flex cursor-pointer items-center gap-1 hover:text-blue-500">
                        {iconName && <DynamicIcon name={iconName} />}
                        {item.name}
                        <DownOutlined className="text-[10px]" />
                    </span>
                </Dropdown>
            );
        }

        if (isLast) {
            return (
                <span className="inline-flex items-center gap-1">
                    {iconName && <DynamicIcon name={iconName} />}
                    {item.name}
                </span>
            );
        }

        if (item.full_path && item.type === 'page') {
            return (
                <span
                    className="inline-flex cursor-pointer items-center gap-1 hover:text-blue-500"
                    onClick={() => navigate(item.full_path!)}
                >
                    {iconName && <DynamicIcon name={iconName} />}
                    {item.name}
                </span>
            );
        }

        return (
            <span className="inline-flex items-center gap-1">
                {iconName && <DynamicIcon name={iconName} />}
                {item.name}
            </span>
        );
    };

    return (
        <Breadcrumb
            items={breadcrumbData.map((data) => ({
                title: renderBreadcrumbItem(data),
            }))}
        />
    );
}
