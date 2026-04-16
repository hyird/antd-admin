import { createMiddleware } from 'hono/factory';
import jwt from 'jsonwebtoken';
import { verifyAccessToken } from '@/utils/jwt';
import { throwAppError } from '@/common/http';
import { AuthError } from '@/modules/system/auth/auth.error';

/**
 * JWT 认证中间件
 */
export const authMiddleware = createMiddleware(async (c, next) => {
    const authHeader = c.req.header('authorization');

    if (!authHeader?.startsWith('Bearer ')) {
        throwAppError(AuthError.UNAUTHORIZED);
    }

    const token = authHeader.slice(7);

    try {
        const payload = verifyAccessToken(token);
        c.set('jwt', payload);
        await next();
    } catch (err) {
        if (err instanceof jwt.TokenExpiredError) {
            throwAppError(AuthError.TOKEN_EXPIRED);
        }
        if (err instanceof jwt.JsonWebTokenError) {
            throwAppError(AuthError.TOKEN_INVALID);
        }
        throw err;
    }
});
