import { Hono } from 'hono';
import { authMiddleware } from '@/middleware/auth';
import { requirePermission, requireAnyPermission } from '@/middleware/permission';
import { menuService } from './menu.service';
import { permissionService } from '@/modules/system/auth/auth.service';
import { R } from '@/common/http';
import { parseBody, parseParams, parseQuery } from '@/common/request';
import { idParamSchema } from '@/common/types';
import type {
    CreateMenuDto,
    UpdateMenuDto,
    ReorderMenuDto,
    BatchCreateMenuButtonsDto,
} from './menu.types';
import type { MenuStatus } from './menu.entity';
import { AppEnv } from '@/core/hono.env';
import {
    batchCreateMenuButtonsSchema,
    createMenuSchema,
    menuQuerySchema,
    reorderMenuSchema,
    updateMenuSchema,
} from './menu.schema';

export const menuRoute = new Hono<AppEnv>();

menuRoute.use('*', authMiddleware);

// 查询菜单列表 - 需要 system:menu:query 权限
menuRoute.get('/', requirePermission('system:menu:query'), async (c) => {
    const query = parseQuery(menuQuerySchema, {
        keyword: c.req.query('keyword'),
        status: c.req.query('status') as MenuStatus | undefined,
    });
    const data = await menuService.listAll(query.keyword);
    return R.ok(c, data);
});

// 查询菜单树 - 需要 system:menu:query 或 system:role:perm 权限
// 菜单管理需要查看菜单树，角色权限配置也需要
menuRoute.get(
    '/tree',
    requireAnyPermission(['system:menu:query', 'system:role:perm']),
    async (c) => {
        const query = parseQuery(menuQuerySchema, {
            status: c.req.query('status') as MenuStatus | undefined,
        });
        const status = query.status;
        const data = await menuService.getTree(status);
        return R.ok(c, data);
    }
);

// 查询菜单详情 - 需要 system:menu:query 权限
menuRoute.get('/:id', requirePermission('system:menu:query'), async (c) => {
    const { id } = parseParams(c, idParamSchema);
    const data = await menuService.getDetail(id);
    return R.ok(c, data);
});

// 新增菜单 - 需要 system:menu:add 权限
menuRoute.post('/', requirePermission('system:menu:add'), async (c) => {
    const body = await parseBody<CreateMenuDto>(c, createMenuSchema);
    await menuService.create(body);
    permissionService.clearAllCache();
    return R.created(c);
});

// 编辑菜单 - 需要 system:menu:edit 权限
menuRoute.put('/:id', requirePermission('system:menu:edit'), async (c) => {
    const { id } = parseParams(c, idParamSchema);
    const body = await parseBody<UpdateMenuDto>(c, updateMenuSchema);
    await menuService.update(id, body);
    permissionService.clearAllCache();
    return R.updated(c);
});

// 菜单排序 - 需要 system:menu:edit 权限
menuRoute.post('/reorder', requirePermission('system:menu:edit'), async (c) => {
    const body = await parseBody<ReorderMenuDto>(c, reorderMenuSchema);
    await menuService.reorder(body);
    permissionService.clearAllCache();
    return R.updated(c);
});

// 批量新增页面按钮权限 - 需要 system:menu:add 权限
menuRoute.post('/batch-buttons', requirePermission('system:menu:add'), async (c) => {
    const body = await parseBody<BatchCreateMenuButtonsDto>(c, batchCreateMenuButtonsSchema);
    const createdCount = await menuService.batchCreateButtons(body);
    permissionService.clearAllCache();
    return R.created(c, { created_count: createdCount });
});

// 删除菜单 - 需要 system:menu:delete 权限
menuRoute.delete('/:id', requirePermission('system:menu:delete'), async (c) => {
    const { id } = parseParams(c, idParamSchema);
    await menuService.remove(id);
    permissionService.clearAllCache();
    return R.deleted(c);
});
