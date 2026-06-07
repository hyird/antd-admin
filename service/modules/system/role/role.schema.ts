import { z } from 'zod';
import { pageParamsSchema } from '../../../common/types.js';

export const createRoleSchema = z.object({
    code: z.string().min(1, '角色编码不能为空').max(50, '角色编码最多50个字符'),
    name: z.string().min(1, '角色名称不能为空').max(100, '角色名称最多100个字符'),
    status: z.enum(['enabled', 'disabled']).optional(),
    menu_ids: z.array(z.number().int().positive()).optional(),
});

export const updateRoleSchema = z.object({
    name: z.string().min(1, '角色名称不能为空').max(100, '角色名称最多100个字符').optional(),
    status: z.enum(['enabled', 'disabled']).optional(),
    menu_ids: z.array(z.number().int().positive()).optional(),
});

export const roleQuerySchema = pageParamsSchema.extend({
    status: z.enum(['enabled', 'disabled']).optional(),
});

export type CreateRoleInput = z.infer<typeof createRoleSchema>;
export type UpdateRoleInput = z.infer<typeof updateRoleSchema>;
export type RoleQueryInput = z.infer<typeof roleQuerySchema>;
