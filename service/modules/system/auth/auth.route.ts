// system/auth/auth.route.ts
import { Hono } from 'hono';
import { authService } from './auth.service';
import { authMiddleware } from './auth.middleware';
import { R } from '@/modules/common/http';
import { AppEnv } from '@/core/hono.env';
import type { LoginRequest, RefreshRequest } from './auth.types';

export const authRoute = new Hono<AppEnv>();

authRoute.post('/login', async (c) => {
    const body = await c.req.json<LoginRequest>();
    const data = await authService.login(body);
    return R.ok(c, data);
});

authRoute.post('/refresh', async (c) => {
    const body = await c.req.json<RefreshRequest>();
    const data = await authService.refresh(body.refreshToken);
    return R.ok(c, data);
});

authRoute.post('/logout', async (c) => {
    await authService.logout();
    return R.ok(c, null, '退出成功');
});

authRoute.get('/me', authMiddleware, async (c) => {
    const { userId } = c.get('jwt');
    const data = await authService.getCurrentUser(userId);
    return R.ok(c, data);
});
