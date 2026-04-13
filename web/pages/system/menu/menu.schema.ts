import { z } from 'zod';
import { pageParamsSchema } from '@/utils/types';

export const createMenuSchema = z.object({
    name: z.string().min(1, '菜单名称不能为空').max(100, '菜单名称最多100个字符'),
    path: z.string().max(200, '路径最多200个字符').optional().nullable(),
    icon: z.string().max(100, '图标最多100个字符').optional(),
    parent_id: z.number().int().positive().optional().nullable(),
    order: z.number().int().min(0).optional(),
    type: z.enum(['menu', 'page', 'button']).optional(),
    component: z.string().max(200, '组件路径最多200个字符').optional(),
    status: z.enum(['enabled', 'disabled']).optional(),
    permission_code: z.string().max(100, '权限编码最多100个字符').optional(),
    is_default: z.boolean().optional(),
});

export const updateMenuSchema = z.object({
    name: z.string().min(1, '菜单名称不能为空').max(100, '菜单名称最多100个字符').optional(),
    path: z.string().max(200, '路径最多200个字符').optional().nullable(),
    icon: z.string().max(100, '图标最多100个字符').optional(),
    parent_id: z.number().int().positive().optional().nullable(),
    order: z.number().int().min(0).optional(),
    type: z.enum(['menu', 'page', 'button']).optional(),
    component: z.string().max(200, '组件路径最多200个字符').optional(),
    status: z.enum(['enabled', 'disabled']).optional(),
    permission_code: z.string().max(100, '权限编码最多100个字符').optional(),
    is_default: z.boolean().optional(),
});

export const menuQuerySchema = pageParamsSchema.extend({
    status: z.enum(['enabled', 'disabled']).optional(),
    type: z.enum(['menu', 'page', 'button']).optional(),
});

export type CreateMenuInput = z.infer<typeof createMenuSchema>;
export type UpdateMenuInput = z.infer<typeof updateMenuSchema>;
export type MenuQueryInput = z.infer<typeof menuQuerySchema>;
