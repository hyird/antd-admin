import jwt, { type SignOptions } from 'jsonwebtoken';
import '@/core/env';
import type { JwtPayload } from '@/modules/system/auth/auth.types';

const JWT_EXPIRES_IN: SignOptions['expiresIn'] =
    (process.env.JWT_EXPIRES_IN as SignOptions['expiresIn']) || '1d';
const JWT_REFRESH_EXPIRES_IN: SignOptions['expiresIn'] =
    (process.env.JWT_REFRESH_EXPIRES_IN as SignOptions['expiresIn']) || '7d';

const JWT_SECRET =
    process.env.JWT_SECRET ??
    (() => {
        throw new Error('JWT_SECRET environment variable is required');
    })();
const JWT_REFRESH_SECRET = process.env.JWT_REFRESH_SECRET || JWT_SECRET;

export function signAccessToken(payload: JwtPayload): string {
    return jwt.sign(payload, JWT_SECRET, { expiresIn: JWT_EXPIRES_IN });
}

export function verifyAccessToken(token: string): JwtPayload {
    const decoded = jwt.verify(token, JWT_SECRET);
    return decoded as JwtPayload;
}

export function signRefreshToken(payload: JwtPayload): string {
    return jwt.sign(payload, JWT_REFRESH_SECRET, {
        expiresIn: JWT_REFRESH_EXPIRES_IN,
    });
}

export function verifyRefreshToken(token: string): JwtPayload {
    const decoded = jwt.verify(token, JWT_REFRESH_SECRET);
    return decoded as JwtPayload;
}
