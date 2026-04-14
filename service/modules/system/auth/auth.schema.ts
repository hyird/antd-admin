import { z } from 'zod';

export const loginSchema = z.object({
    username: z.string().min(1, '用户名不能为空'),
    password: z.string().min(1, '密码不能为空'),
});

export const refreshSchema = z.object({
    refresh_token: z.string().min(1, '刷新令牌不能为空'),
});

export const logoutSchema = z.object({
    refresh_token: z.string().optional(),
});

export type LoginInput = z.infer<typeof loginSchema>;
export type RefreshInput = z.infer<typeof refreshSchema>;
export type LogoutInput = z.infer<typeof logoutSchema>;
