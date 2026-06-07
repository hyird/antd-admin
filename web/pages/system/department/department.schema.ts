import { z } from 'zod';
import { pageParamsSchema } from '@/utils/types';

export const createDepartmentSchema = z.object({
    name: z.string().min(1, '部门名称不能为空').max(100, '部门名称最多100个字符'),
    code: z.string().max(50, '部门编码最多50个字符').optional(),
    parent_id: z.number().int().positive().optional().nullable(),
    sort_order: z.number().int().min(0).optional(),
    leader_id: z.number().int().positive().optional().nullable(),
    status: z.enum(['enabled', 'disabled']).optional(),
});

export const updateDepartmentSchema = z.object({
    name: z.string().min(1, '部门名称不能为空').max(100, '部门名称最多100个字符').optional(),
    code: z.string().max(50, '部门编码最多50个字符').optional(),
    parent_id: z.number().int().positive().optional().nullable(),
    sort_order: z.number().int().min(0).optional(),
    leader_id: z.number().int().positive().optional().nullable(),
    status: z.enum(['enabled', 'disabled']).optional(),
});

export const departmentQuerySchema = pageParamsSchema.extend({
    status: z.enum(['enabled', 'disabled']).optional(),
});

export type CreateDepartmentInput = z.infer<typeof createDepartmentSchema>;
export type UpdateDepartmentInput = z.infer<typeof updateDepartmentSchema>;
export type DepartmentQueryInput = z.infer<typeof departmentQuerySchema>;
