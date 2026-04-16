// system/auth/auth.route.ts
import { Hono } from 'hono';
import { authService } from './auth.service';
import { authMiddleware } from '@/middleware/auth';
import { R } from '@/common/http';
import { parseBody } from '@/common/request';
import type { AppEnv } from '@/core/hono.env';
import type { LoginRequest, RefreshRequest } from './auth.types';
import { loginSchema, refreshSchema } from './auth.schema';

export const authRoute = new Hono<AppEnv>();

authRoute.post('/login', async (c) => {
    const body = await parseBody<LoginRequest>(c, loginSchema);
    const data = await authService.login(body);
    return R.ok(c, data);
});

authRoute.post('/refresh', async (c) => {
    const body = await parseBody<RefreshRequest>(c, refreshSchema);
    const data = await authService.refresh(body.refresh_token);
    return R.ok(c, data);
});

authRoute.post('/logout', async (c) => {
    await authService.logout();
    return R.ok(c, null, '退出成功');
});

authRoute.get('/me', authMiddleware, async (c) => {
    const { user_id } = c.get('jwt');
    const data = await authService.getCurrentUser(user_id);
    return R.ok(c, data);
});
