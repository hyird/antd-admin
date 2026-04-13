import { Hono } from 'hono';
import { authMiddleware } from '@/modules/system/auth/auth.middleware';
import {
    requirePermission,
    requireAnyPermission,
} from '@/modules/system/auth/permission.middleware';
import { departmentService } from './department.service';
import { R } from '@/modules/common/http';
import type { CreateDepartmentDto, UpdateDepartmentDto } from './department.types';
import type { DepartmentStatus } from './department.entity';
import { AppEnv } from '@/core/hono.env';

export const departmentRoute = new Hono<AppEnv>();

departmentRoute.use('*', authMiddleware);

// 查询部门列表 - 需要 system:dept:query 权限
departmentRoute.get('/', requirePermission('system:dept:query'), async (c) => {
    const keyword = c.req.query('keyword');
    const data = await departmentService.listAll(keyword);
    return R.ok(c, data);
});

// 查询部门树 - 需要 system:dept:query 或 system:user:add 或 system:user:edit 权限
// 部门管理需要查看部门树，用户管理选择部门也需要
departmentRoute.get(
    '/tree',
    requireAnyPermission(['system:dept:query', 'system:user:add', 'system:user:edit']),
    async (c) => {
        const status = c.req.query('status') as DepartmentStatus | undefined;
        const data = await departmentService.getTree(status);
        return R.ok(c, data);
    }
);

// 查询部门详情 - 需要 system:dept:query 权限
departmentRoute.get('/:id', requirePermission('system:dept:query'), async (c) => {
    const id = Number(c.req.param('id'));
    const data = await departmentService.getById(id);
    return R.ok(c, data);
});

// 新增部门 - 需要 system:dept:add 权限
departmentRoute.post('/', requirePermission('system:dept:add'), async (c) => {
    const body = await c.req.json<CreateDepartmentDto>();
    await departmentService.create(body);
    return R.created(c);
});

// 编辑部门 - 需要 system:dept:edit 权限
departmentRoute.put('/:id', requirePermission('system:dept:edit'), async (c) => {
    const id = Number(c.req.param('id'));
    const body = await c.req.json<UpdateDepartmentDto>();
    await departmentService.update(id, body);
    return R.updated(c);
});

// 删除部门 - 需要 system:dept:delete 权限
departmentRoute.delete('/:id', requirePermission('system:dept:delete'), async (c) => {
    const id = Number(c.req.param('id'));
    await departmentService.remove(id);
    return R.deleted(c);
});
