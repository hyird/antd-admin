import type { MenuItem } from '../menu/menu.types';
import type { RoleOption } from '../role/role.types';

export interface JwtPayload {
    userId: number;
    username: string;
    iat?: number;
    exp?: number;
}

export interface LoginRequest {
    username: string;
    password: string;
}

export interface RefreshRequest {
    refreshToken: string;
}

export interface RefreshResult {
    token: string;
    refreshToken: string;
    user: UserInfo;
}

export interface LoginResult {
    token: string;
    refreshToken: string;
    user: UserInfo;
}

export interface UserInfo {
    id: number;
    username: string;
    nickname?: string;
    status: string;
    roles: RoleOption[];
    menus: MenuItem[];
}
