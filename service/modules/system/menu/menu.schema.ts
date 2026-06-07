import { z } from 'zod';
import { pageParamsSchema } from '../../../common/types.js';

export const createMenuSchema = z.object({
    name: z.string().min(1, '菜单名称不能为空').max(100, '菜单名称最多100个字符'),
    path: z.string().max(200, '路径最多200个字符').optional().nullable(),
    icon: z.string().max(100, '图标最多100个字符').optional(),
    parent_id: z.number().int().positive().optional().nullable(),
    sort_order: z.number().int().min(0).optional(),
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
    sort_order: z.number().int().min(0).optional(),
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

export const reorderMenuSchema = z.object({
    items: z.array(
        z.object({
            id: z.number().int().positive(),
            sort_order: z.number().int().min(0),
            parent_id: z.number().int().positive().optional().nullable(),
        })
    ),
});

export const batchCreateMenuButtonsSchema = z.object({
    parent_id: z.number().int().positive(),
    items: z.array(
        z.object({
            name: z.string().min(1, '按钮名称不能为空').max(100, '按钮名称最多100个字符'),
            permission_code: z
                .string()
                .min(1, '权限编码不能为空')
                .max(100, '权限编码最多100个字符'),
        })
    ),
});

export type CreateMenuInput = z.infer<typeof createMenuSchema>;
export type UpdateMenuInput = z.infer<typeof updateMenuSchema>;
export type MenuQueryInput = z.infer<typeof menuQuerySchema>;
export type ReorderMenuInput = z.infer<typeof reorderMenuSchema>;
export type BatchCreateMenuButtonsInput = z.infer<typeof batchCreateMenuButtonsSchema>;
