import { Hono } from 'hono';
import { authMiddleware } from '../../../middleware/auth.js';
import { requirePermission, requireAnyPermission } from '../../../middleware/permission.js';
import { roleService } from './role.service';
import { permissionService } from '../auth/auth.service.js';
import { R } from '../../../common/http.js';
import { getQuery, parseBody, parseParams, parseQuery } from '../../../common/request.js';
import { idParamSchema } from '../../../common/types.js';
import type { CreateRoleDto, UpdateRoleDto, RoleStatus } from './role.types';
import type { AppEnv } from '../../../core/hono.env.js';
import { createRoleSchema, roleQuerySchema, updateRoleSchema } from './role.schema';

export const roleRoute = new Hono<AppEnv>();

roleRoute.use('*', authMiddleware);

// 查询角色列表 - 需要 system:role:query 权限
roleRoute.get('/', requirePermission('system:role:query'), async (c) => {
    const query = parseQuery(roleQuerySchema, {
        page: getQuery(c, 'page'),
        pageSize: getQuery(c, 'pageSize', { aliases: ['page_size'] }),
        keyword: getQuery(c, 'keyword'),
        status: getQuery(c, 'status') as RoleStatus | undefined,
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
