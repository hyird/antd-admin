import { Hono } from 'hono';
import { authMiddleware } from '@/middleware/auth';
import { requireAnyPermission, requirePermission } from '@/middleware/permission';
import { userService } from './user.service';
import { R } from '@/common/http';
import { parseBody, parseParams, parseQuery } from '@/common/request';
import { idParamSchema } from '@/common/types';
import type { CreateUserDto, UpdateUserDto, UserStatus } from './user.types';
import type { AppEnv } from '@/core/hono.env';
import { createUserSchema, updateUserSchema, userQuerySchema } from './user.schema';

export const userRoute = new Hono<AppEnv>();

userRoute.use('*', authMiddleware);

// 查询用户列表 - 需要 system:user:query 权限
userRoute.get('/', requirePermission('system:user:query'), async (c) => {
    const query = parseQuery(userQuerySchema, {
        page: c.req.query('page'),
        pageSize: c.req.query('pageSize') ?? c.req.query('page_size'),
        keyword: c.req.query('keyword'),
        status: c.req.query('status') as UserStatus | undefined,
        department_id: c.req.query('department_id')
            ? Number(c.req.query('department_id'))
            : undefined,
    });
    const data = await userService.list(query);
    return R.page(c, data);
});

// 用户选项列表（部门负责人选择器等）
userRoute.get(
    '/options',
    requireAnyPermission([
        'system:user:query',
        'system:user:add',
        'system:user:edit',
        'system:dept:query',
        'system:dept:add',
        'system:dept:edit',
    ]),
    async (c) => {
        const keyword = c.req.query('keyword');
        const data = await userService.listOptions(keyword);
        return R.ok(c, data);
    }
);

// 查询用户详情 - 需要 system:user:query 权限
userRoute.get('/:id', requirePermission('system:user:query'), async (c) => {
    const { id } = parseParams(c, idParamSchema);
    const data = await userService.getById(id);
    return R.ok(c, data);
});

// 新增用户 - 需要 system:user:add 权限
userRoute.post('/', requirePermission('system:user:add'), async (c) => {
    const body = await parseBody<CreateUserDto>(c, createUserSchema);
    await userService.create(body);
    return R.created(c);
});

// 编辑用户 - 需要 system:user:edit 权限
userRoute.put('/:id', requirePermission('system:user:edit'), async (c) => {
    const { id } = parseParams(c, idParamSchema);
    const body = await parseBody<UpdateUserDto>(c, updateUserSchema);
    await userService.update(id, body);
    return R.updated(c);
});

// 删除用户 - 需要 system:user:delete 权限
userRoute.delete('/:id', requirePermission('system:user:delete'), async (c) => {
    const { id } = parseParams(c, idParamSchema);
    await userService.remove(id);
    return R.deleted(c);
});
