import type { MenuItem } from '../menu/menu.types';
import type { RoleOption } from '../role/role.types';

export interface JwtPayload {
    user_id: number;
    username: string;
    iat?: number;
    exp?: number;
}

export interface LoginRequest {
    username: string;
    password: string;
}

export interface RefreshRequest {
    refresh_token: string;
}

export interface RefreshResult {
    token: string;
    refresh_token: string;
    user: UserInfo;
}

export interface LoginResult {
    token: string;
    refresh_token: string;
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
