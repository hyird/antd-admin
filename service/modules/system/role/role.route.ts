import { Hono } from 'hono';
import { authMiddleware } from '@/modules/system/auth/auth.middleware';
import {
    requirePermission,
    requireAnyPermission,
} from '@/modules/system/auth/permission.middleware';
import { roleService } from './role.service';
import { permissionService } from '@/modules/system/auth/permission.service';
import { R } from '@/modules/common/http';
import { parseBody, parseParams, parseQuery } from '@/modules/common/request';
import { idParamSchema } from '@/modules/common/types';
import type { CreateRoleDto, UpdateRoleDto, RoleStatus } from './role.types';
import { AppEnv } from '@/core/hono.env';
import { createRoleSchema, roleQuerySchema, updateRoleSchema } from './role.schema';

export const roleRoute = new Hono<AppEnv>();

roleRoute.use('*', authMiddleware);

// 查询角色列表 - 需要 system:role:query 权限
roleRoute.get('/', requirePermission('system:role:query'), async (c) => {
    const query = parseQuery(roleQuerySchema, {
        page: c.req.query('page'),
        pageSize: c.req.query('pageSize') ?? c.req.query('page_size'),
        keyword: c.req.query('keyword'),
        status: c.req.query('status') as RoleStatus | undefined,
    });
    const data = await roleService.list(query);
    return R.page(c, data);
});

// 角色选择器（用户管理分配角色时使用）- 需要 system:user:add 或 system:user:edit 权限
roleRoute.get('/all', requireAnyPermission(['system:user:add', 'system:user:edit']), async (c) => {
    const data = await roleService.listAllEnabled();
    return R.ok(c, data);
});

// 查询角色详情 - 需要 system:role:query 权限
roleRoute.get('/:id', requirePermission('system:role:query'), async (c) => {
    const { id } = parseParams(c, idParamSchema);
    const data = await roleService.getById(id);
    return R.ok(c, data);
});

// 新增角色 - 需要 system:role:add 权限
roleRoute.post('/', requirePermission('system:role:add'), async (c) => {
    const body = await parseBody<CreateRoleDto>(c, createRoleSchema);
    await roleService.create(body);
    return R.created(c);
});

// 编辑角色 - 需要 system:role:edit 权限
roleRoute.put('/:id', requirePermission('system:role:edit'), async (c) => {
    const { id } = parseParams(c, idParamSchema);
    const body = await parseBody<UpdateRoleDto>(c, updateRoleSchema);
    await roleService.update(id, body);
    permissionService.clearAllCache();
    return R.updated(c);
});

// 删除角色 - 需要 system:role:delete 权限
roleRoute.delete('/:id', requirePermission('system:role:delete'), async (c) => {
    const { id } = parseParams(c, idParamSchema);
    await roleService.remove(id);
    permissionService.clearAllCache();
    return R.deleted(c);
});
