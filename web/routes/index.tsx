import '@/pages';
import { type ComponentType, type ReactNode, lazy, useMemo, useRef } from 'react';
import {
    createHashRouter,
    Navigate,
    Outlet,
    RouterProvider,
    useLocation,
    useNavigate,
    useOutlet,
} from 'react-router-dom';
import { Button, Result } from 'antd';
import { AnimatePresence, domAnimation, LazyMotion, m } from 'framer-motion';
import { fadeVariants, pageTransition } from '@/utils/animations';
import { getComponentLoaderMap } from '@/pages';
import { useDynamicRoutes } from '@/hooks/useDynamicRoutes';
import { useInitAuth } from '@/hooks/useInitAuth';
import { useAuthStore } from '@/store/authStore';
import { APP_NAME, getAppTitle } from '@/config/app';
import type { Menu } from '@/pages/system/menu/menu.types';

function AuthGuard() {
    const token = useAuthStore((s) => s.token);
    const location = useLocation();
    const redirectState = useMemo(
        () => ({ from: { pathname: location.pathname } }),
        [location.pathname]
    );
    if (!token) {
        return <Navigate to="/login" state={redirectState} replace />;
    }
    return <Outlet />;
}

function RootTransition() {
    const outlet = useOutlet();
    const location = useLocation();
    const animationKey = location.pathname === '/login' ? 'login' : 'app';
    return (
        <LazyMotion features={domAnimation}>
            <AnimatePresence mode="wait" initial={false}>
                <m.div
                    key={animationKey}
                    initial="initial"
                    animate="animate"
                    exit="exit"
                    variants={fadeVariants}
                    transition={pageTransition}
                    className="h-full w-full"
                >
                    {outlet}
                </m.div>
            </AnimatePresence>
        </LazyMotion>
    );
}

function FallbackPage({ menu }: { menu: Menu.Item }) {
    const navigate = useNavigate();
    return (
        <Result
            status="info"
            title={getAppTitle(`「${menu.name}」页面建设中`)}
            subTitle={
                <>
                    系统名称：<code>{APP_NAME}</code>
                    <br />
                    路由地址：<code>{menu.path}</code>
                    <br />
                    组件配置：<code>{menu.component ?? '未配置'}</code>
                </>
            }
            extra={
                <Button type="primary" onClick={() => navigate('/')}>
                    返回首页
                </Button>
            }
        />
    );
}

// 固定的页面
const LoginPage = lazy(() => import('@/pages/login'));
const AdminLayout = lazy(() => import('@/layouts/AdminLayout'));
const HomePage = lazy(() => import('@/pages/home'));

// 从统一注册中心获取组件映射
const componentMap: Record<string, () => Promise<{ default: ComponentType<unknown> }>> =
    getComponentLoaderMap();

// 懒加载缓存
const lazyCache = new Map<string, ComponentType<unknown>>();

function getLazyComponent(name: string): ComponentType<unknown> | null {
    const loader = componentMap[name];
    if (!loader) {
        return null;
    }
    if (!lazyCache.has(name)) {
        lazyCache.set(name, lazy(loader));
    }
    // biome-ignore lint/style/noNonNullAssertion: lazyCache.has check above guarantees value exists
    return lazyCache.get(name)!;
}

function getRouteComponent(menu: Menu.Item): ReactNode {
    if (menu.component) {
        const Component = getLazyComponent(menu.component);
        if (Component) {
            return <Component />;
        }
    }
    return <FallbackPage menu={menu} />;
}

export function AppRoutes() {
    useInitAuth();
    const { pageMenus: rawPageMenus } = useDynamicRoutes();

    const pageMenusRef = useRef(rawPageMenus);
    const pageMenusKeyRef = useRef('');
    const newKey = rawPageMenus.map((m) => `${m.path}:${m.component}`).join('|');
    if (newKey !== pageMenusKeyRef.current) {
        pageMenusKeyRef.current = newKey;
        pageMenusRef.current = rawPageMenus;
    }
    const pageMenus = pageMenusRef.current;

    const router = useMemo(
        () =>
            createHashRouter([
                {
                    element: <RootTransition />,
                    children: [
                        {
                            path: '/login',
                            element: <LoginPage />,
                        },
                        {
                            element: <AuthGuard />,
                            children: [
                                {
                                    path: '/',
                                    element: <AdminLayout />,
                                    children: [
                                        {
                                            index: true,
                                            element: <Navigate to="/home" replace />,
                                        },
                                        {
                                            path: 'home',
                                            element: <HomePage />,
                                        },
                                        ...pageMenus.map((menu) => ({
                                            path: menu.path?.trim().replace(/^\//, ''),
                                            element: getRouteComponent(menu),
                                        })),
                                    ],
                                },
                            ],
                        },
                        {
                            path: '*',
                            element: <Navigate to="/" replace />,
                        },
                    ],
                },
            ]),
        [pageMenus]
    );

    return <RouterProvider router={router} />;
}
