import type { PageParams } from '@/modules/common/types';

export type UserStatus = 'enabled' | 'disabled';

export interface UserRole {
    id: number;
    name: string;
    code: string;
}

export interface UserOption {
    id: number;
    username: string;
    nickname?: string;
    phone?: string;
    email?: string;
}

export interface UserItem {
    id: number;
    username: string;
    nickname?: string;
    phone?: string;
    email?: string;
    department_id?: number | null;
    departmentName?: string;
    status: UserStatus;
    roles: UserRole[];
    created_at?: string;
    updated_at?: string;
}

export interface UserQuery extends PageParams {
    status?: UserStatus;
    department_id?: number;
}

export interface CreateUserDto {
    username: string;
    password: string;
    nickname?: string;
    phone?: string;
    email?: string;
    department_id?: number | null;
    status?: UserStatus;
    roleIds?: number[];
}

export interface UpdateUserDto {
    nickname?: string;
    phone?: string;
    email?: string;
    department_id?: number | null;
    status?: UserStatus;
    password?: string;
    roleIds?: number[];
}

export interface UpdatePasswordDto {
    oldPassword: string;
    newPassword: string;
}
