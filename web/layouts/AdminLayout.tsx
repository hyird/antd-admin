import {
    LogoutOutlined,
    MenuFoldOutlined,
    MenuUnfoldOutlined,
    SearchOutlined,
    UserOutlined,
} from '@ant-design/icons';
import type { MenuProps } from 'antd';
import { Menu, Button, Dropdown, Input, Layout, Space, Spin } from 'antd';
import type { ItemType } from 'antd/es/menu/interface';
import { lazy, Suspense, useDeferredValue, useMemo, useState } from 'react';
import { useLocation, useNavigate, useOutlet } from 'react-router-dom';
import { AnimatePresence, domAnimation, LazyMotion, m } from 'framer-motion';
import { APP_NAME } from '@/config/app';
import { useLogout, useCurrentUser } from '@/pages/login';
import { useAuthStore } from '@/store/authStore';
import { useTabsStore } from '@/store/tabsStore';
import type { MenuTreeItem } from '@/pages/system/menu/menu.types';
import { buildMenuTree } from '@/utils/tree';
import { renderMenuIcon } from '@/utils/icon';
import { fastTransition, slideUpVariants } from '@/utils/animations';

const { Header, Sider, Content } = Layout;
const USER_MENU_WIDTH = 192;
const Breadcrumb = lazy(() => import('@/components/Breadcrumb'));
const PageTabs = lazy(() => import('@/components/PageTabs'));

function PageTransition() {
    const outlet = useOutlet();
    const location = useLocation();
    return (
        <LazyMotion features={domAnimation}>
            <AnimatePresence mode="wait" initial={false}>
                <m.div
                    key={location.pathname}
                    initial="initial"
                    animate="animate"
                    exit="exit"
                    variants={slideUpVariants}
                    transition={fastTransition}
                    className="h-full w-full"
                >
                    {outlet}
                </m.div>
            </AnimatePresence>
        </LazyMotion>
    );
}

function isDisplayMenuItem(type: MenuTreeItem['type']) {
    return type === 'menu' || type === 'page';
}

function buildMenuItems(items: MenuTreeItem[]): ItemType[] {
    return items
        .filter((item) => isDisplayMenuItem(item.type))
        .map((item) => {
            const children = item.children?.filter((child) => isDisplayMenuItem(child.type));
            const key = item.full_path || item.path || `menu-${item.id}`;
            const baseItem = {
                key,
                icon: renderMenuIcon(item),
                label: item.name,
            };

            if (children && children.length > 0) {
                return { ...baseItem, children: buildMenuItems(children) };
            }

            if (item.type === 'menu') {
                return { ...baseItem, key: `menu-${item.id}` };
            }

            return baseItem;
        });
}

function filterMenuItems(items: ItemType[], keyword: string): ItemType[] {
    const normalizedKeyword = keyword.trim().toLowerCase();
    if (!normalizedKeyword) return items;

    // 保留命中的父节点，避免搜索后整棵菜单树失去上下文。
    return items
        .map((item) => {
            if (!item || !('key' in item)) return null;
            const menuItem = item as {
                key: string;
                label?: string;
                children?: ItemType[];
            };

            if (menuItem.children?.length) {
                const filteredChildren = filterMenuItems(menuItem.children, normalizedKeyword);
                if (filteredChildren.length > 0) {
                    return { ...menuItem, children: filteredChildren };
                }
            }

            const label = typeof menuItem.label === 'string' ? menuItem.label : '';
            if (label.toLowerCase().includes(normalizedKeyword)) return item;

            return null;
        })
        .filter(Boolean) as ItemType[];
}

export default function AdminLayout() {
    const { token } = useAuthStore();
    const { clearTabs } = useTabsStore();
    const { data: user } = useCurrentUser();
    const logoutMutation = useLogout();
    const navigate = useNavigate();
    const location = useLocation();

    const [collapsed, setCollapsed] = useState(false);
    const [menuSearch, setMenuSearch] = useState('');
    const deferredMenuSearch = useDeferredValue(menuSearch);

    // 有 token 但完全没有 user 缓存时才显示 loading
    const isInitialLoading = !!token && !user;

    const menuTree = useMemo<MenuTreeItem[]>(() => {
        const menus = user?.menus || [];
        return buildMenuTree(menus);
    }, [user?.menus]);

    const menuItems = useMemo(() => {
        return buildMenuItems(menuTree);
    }, [menuTree]);

    // 菜单搜索过滤
    const filteredMenuItems = useMemo(() => {
        return filterMenuItems(menuItems, deferredMenuSearch);
    }, [menuItems, deferredMenuSearch]);

    const onMenuClick = (info: { key: string }) => {
        // 空目录菜单（key 以 menu- 开头）点击无反应
        if (info.key.startsWith('menu-')) return;
        navigate(info.key);
    };

    const userMenu: MenuProps = {
        style: { width: USER_MENU_WIDTH },
        items: [
            {
                key: 'logout',
                icon: <LogoutOutlined />,
                label: '退出登录',
            },
        ],
        onClick: (info) => {
            if (info.key === 'logout') {
                clearTabs();
                logoutMutation.mutate();
            }
        },
    };

    // 侧边栏菜单内容
    const siderContent = (
        <>
            <div className="m-2 flex h-12 shrink-0 items-center justify-center rounded font-medium text-white">
                {!isInitialLoading && <span>{APP_NAME}</span>}
            </div>
            {!isInitialLoading && (
                <>
                    {!collapsed && (
                        <div className="mb-2 px-3">
                            <Input
                                allowClear
                                prefix={<SearchOutlined className="!text-white/30" />}
                                placeholder="搜索菜单"
                                variant="borderless"
                                value={menuSearch}
                                onChange={(e) => setMenuSearch(e.target.value)}
                                className="sider-search"
                            />
                        </div>
                    )}
                    <div className="scrollbar-none flex-1 overflow-auto">
                        <Menu
                            theme="dark"
                            mode="inline"
                            selectedKeys={[location.pathname]}
                            items={filteredMenuItems}
                            onClick={onMenuClick}
                            inlineCollapsed={collapsed}
                        />
                    </div>
                </>
            )}
        </>
    );

    return (
        <Layout className="h-screen overflow-hidden">
            <Sider
                width={220}
                className="flex flex-col"
                collapsible
                collapsed={collapsed}
                onCollapse={setCollapsed}
                trigger={null}
            >
                {siderContent}
            </Sider>

            <Layout className="flex flex-col overflow-hidden bg-[#f5f5f5]">
                <Header className="relative z-10 flex h-12 shrink-0 items-center justify-between bg-white px-4 shadow-[0_2px_8px_rgba(0,0,0,0.06)]">
                    <div className="flex flex-1 items-center gap-2">
                        <Button
                            type="text"
                            icon={collapsed ? <MenuUnfoldOutlined /> : <MenuFoldOutlined />}
                            onClick={() => setCollapsed(!collapsed)}
                        />
                        {!isInitialLoading && (
                            <Suspense fallback={null}>
                                <Breadcrumb />
                            </Suspense>
                        )}
                    </div>
                    <div className="flex items-center justify-end gap-3">
                        {isInitialLoading ? (
                            <div className="flex items-center gap-2">
                                <UserOutlined />
                            </div>
                        ) : (
                            <Dropdown
                                menu={userMenu}
                                placement="bottomRight"
                                styles={{ root: { width: USER_MENU_WIDTH } }}
                                trigger={['click']}
                            >
                                <Button
                                    className="!inline-flex !items-center !justify-center"
                                    style={{ width: USER_MENU_WIDTH }}
                                >
                                    <Space className="min-w-0">
                                        <UserOutlined />
                                        <span className="max-w-28 truncate">{user?.username}</span>
                                    </Space>
                                </Button>
                            </Dropdown>
                        )}
                    </div>
                </Header>

                {!isInitialLoading && (
                    <Suspense fallback={null}>
                        <PageTabs />
                    </Suspense>
                )}

                <Content className="m-4 flex flex-1 flex-col overflow-hidden rounded-lg bg-white">
                    {isInitialLoading ? (
                        <div className="flex flex-1 items-center justify-center">
                            <Spin tip="加载中..." fullscreen />
                        </div>
                    ) : (
                        <div className="flex flex-1 flex-col overflow-auto">
                            <PageTransition />
                        </div>
                    )}
                </Content>
            </Layout>
        </Layout>
    );
}
