import { repo } from '@/config/data';
import { hashPassword } from '@/utils/bcrypt';
import { throwAppError } from '@/common/http';
import { UserError } from './user.error';
import {
    normalizePagination,
    paginate,
    applyKeywordSearch,
    type QueryResult,
} from '@/common/types';
import type { UserQuery, UserItem, UserOption, CreateUserDto, UpdateUserDto } from './user.types';
import { In, Not, type FindOptionsWhere } from 'typeorm';
import type { User } from './user.entity';
import type { RoleStatus } from '../role/role.entity';
import type { MenuStatus, MenuType } from '../menu/menu.entity';
import { permissionService } from '../auth/auth.service';
import { createLogger } from '@/utils/logger';

const logger = createLogger('user:seed');

const PHONE_REGEX = /^1[3-9]\d{9}$/;
const EMAIL_REGEX = /^[^\s@]+@[^\s@]+\.[^\s@]+$/;

const SYSTEM_MENU_ORDER = 1;
const SYSTEM_PAGE_ORDER = {
    menu: 1,
    department: 2,
    role: 3,
    user: 4,
} as const;

export class UserService {
    private async ensureButtons(
        parentId: number,
        buttons: Array<{ name: string; permission_code: string; order: number }>
    ): Promise<void> {
        if (!buttons.length) {
            return;
        }

        const permissionCodes = buttons.map((btn) => btn.permission_code);
        const existingButtons = await repo.menu.find({
            where: {
                type: 'button' as MenuType,
                parent_id: parentId,
                permission_code: In(permissionCodes),
            },
            select: {
                id: true,
                order: true,
                permission_code: true,
            },
        });
        const existingCodeSet = new Set(
            existingButtons
                .map((item) => item.permission_code)
                .filter((code): code is string => Boolean(code))
        );

        const existingButtonMap = new Map(
            existingButtons
                .filter((item): item is typeof item & { permission_code: string } =>
                    Boolean(item.permission_code)
                )
                .map((item) => [item.permission_code, item])
        );

        for (const btn of buttons) {
            const existingButton = existingButtonMap.get(btn.permission_code);
            if (existingButton && existingButton.order !== btn.order) {
                existingButton.order = btn.order;
                await repo.menu.save(existingButton);
            }
        }

        const missingButtons = buttons.filter((btn) => !existingCodeSet.has(btn.permission_code));
        if (!missingButtons.length) {
            return;
        }

        await repo.menu.save(
            missingButtons.map((btn) =>
                repo.menu.create({
                    name: btn.name,
                    type: 'button' as MenuType,
                    status: 'enabled' as MenuStatus,
                    parent_id: parentId,
                    order: btn.order,
                    permission_code: btn.permission_code,
                })
            )
        );
    }

    private toUserItem(u: User): UserItem {
        return {
            id: u.id,
            username: u.username,
            nickname: u.nickname,
            phone: u.phone,
            email: u.email,
            department_id: u.department_id,
            department_name: u.department?.name,
            status: u.status,
            roles: (u.roles || []).map((r) => ({
                id: r.id,
                name: r.name,
                code: r.code,
            })),
        };
    }

    private toUserOption(u: User): UserOption {
        return {
            id: u.id,
            username: u.username,
            nickname: u.nickname,
            phone: u.phone,
            email: u.email,
        };
    }

    async seedAdmin(): Promise<void> {
        logger.info('Starting admin initialization');

        const systemMenu = await this.ensureSystemMenu();

        const menuPage = await this.ensureMenuPage(systemMenu.id);
        await this.ensureMenuButtons(menuPage.id);

        const deptPage = await this.ensureDeptPage(systemMenu.id);
        await this.ensureDeptButtons(deptPage.id);

        const rolePage = await this.ensureRolePage(systemMenu.id);
        await this.ensureRoleButtons(rolePage.id);

        const userPage = await this.ensureUserPage(systemMenu.id);
        await this.ensureUserButtons(userPage.id);

        const superRole = await this.ensureSuperAdminRole();

        await this.ensureAdminUser(superRole.id);

        logger.info('Admin initialization completed');
    }

    private async ensureSystemMenu() {
        let menu = await repo.menu.findOne({
            where: { path: '/system', type: 'menu' as MenuType },
        });

        if (!menu) {
            menu = await repo.menu.save(
                repo.menu.create({
                    name: '系统管理',
                    path: '/system',
                    icon: 'SettingOutlined',
                    type: 'menu' as MenuType,
                    status: 'enabled' as MenuStatus,
                    order: SYSTEM_MENU_ORDER,
                })
            );
            logger.info(`Created system menu: id=${menu.id}`);
        } else if (menu.order !== SYSTEM_MENU_ORDER) {
            menu.order = SYSTEM_MENU_ORDER;
            await repo.menu.save(menu);
        }

        return menu;
    }

    private async ensureMenuPage(systemMenuId: number) {
        let menuPage = await repo.menu.findOne({
            where: { component: 'Menu' },
        });

        if (menuPage) {
            let needUpdate = false;

            if (!menuPage.path) {
                menuPage.path = '/system/menu';
                needUpdate = true;
            }
            if (menuPage.parent_id !== systemMenuId) {
                menuPage.parent_id = systemMenuId;
                needUpdate = true;
            }
            if (menuPage.type !== 'page') {
                menuPage.type = 'page' as MenuType;
                needUpdate = true;
            }
            if (menuPage.status !== 'enabled') {
                menuPage.status = 'enabled' as MenuStatus;
                needUpdate = true;
            }
            if (menuPage.order !== SYSTEM_PAGE_ORDER.menu) {
                menuPage.order = SYSTEM_PAGE_ORDER.menu;
                needUpdate = true;
            }

            if (needUpdate) {
                await repo.menu.save(menuPage);
                logger.info(`Updated menu page config: id=${menuPage.id}`);
            }

            return menuPage;
        }

        menuPage = await repo.menu.findOne({
            where: { path: '/system/menu', type: 'page' as MenuType },
        });

        if (menuPage) {
            if (!menuPage.component) {
                menuPage.component = 'Menu';
                await repo.menu.save(menuPage);
                logger.info(`Set menu page component: id=${menuPage.id}`);
            }
            return menuPage;
        }

        menuPage = await repo.menu.save(
            repo.menu.create({
                name: '菜单管理',
                path: '/system/menu',
                component: 'Menu',
                type: 'page' as MenuType,
                status: 'enabled' as MenuStatus,
                parent_id: systemMenuId,
                order: SYSTEM_PAGE_ORDER.menu,
            })
        );
        logger.info(`Created menu page: id=${menuPage.id}`);

        return menuPage;
    }

    private async ensureMenuButtons(menuPageId: number) {
        const buttons = [
            { name: '查询菜单', permission_code: 'system:menu:query', order: 1 },
            { name: '新增菜单', permission_code: 'system:menu:add', order: 2 },
            { name: '编辑菜单', permission_code: 'system:menu:edit', order: 3 },
            { name: '删除菜单', permission_code: 'system:menu:delete', order: 4 },
        ];
        await this.ensureButtons(menuPageId, buttons);
    }

    private async ensureUserPage(systemMenuId: number) {
        let userPage = await repo.menu.findOne({
            where: { component: 'User' },
        });

        if (userPage) {
            let needUpdate = false;
            if (!userPage.path) {
                userPage.path = '/system/user';
                needUpdate = true;
            }
            if (userPage.parent_id !== systemMenuId) {
                userPage.parent_id = systemMenuId;
                needUpdate = true;
            }
            if (userPage.type !== 'page') {
                userPage.type = 'page' as MenuType;
                needUpdate = true;
            }
            if (userPage.status !== 'enabled') {
                userPage.status = 'enabled' as MenuStatus;
                needUpdate = true;
            }
            if (userPage.order !== SYSTEM_PAGE_ORDER.user) {
                userPage.order = SYSTEM_PAGE_ORDER.user;
                needUpdate = true;
            }
            if (needUpdate) {
                await repo.menu.save(userPage);
                logger.info(`Updated user page config: id=${userPage.id}`);
            }
            return userPage;
        }

        userPage = await repo.menu.save(
            repo.menu.create({
                name: '用户管理',
                path: '/system/user',
                component: 'User',
                type: 'page' as MenuType,
                status: 'enabled' as MenuStatus,
                parent_id: systemMenuId,
                order: SYSTEM_PAGE_ORDER.user,
            })
        );
        logger.info(`Created user page: id=${userPage.id}`);

        return userPage;
    }

    private async ensureUserButtons(userPageId: number) {
        const buttons = [
            { name: '查询用户', permission_code: 'system:user:query', order: 1 },
            { name: '新增用户', permission_code: 'system:user:add', order: 2 },
            { name: '编辑用户', permission_code: 'system:user:edit', order: 3 },
            { name: '删除用户', permission_code: 'system:user:delete', order: 4 },
            { name: '重置密码', permission_code: 'system:user:reset', order: 5 },
        ];
        await this.ensureButtons(userPageId, buttons);
    }

    private async ensureRolePage(systemMenuId: number) {
        let rolePage = await repo.menu.findOne({
            where: { component: 'Role' },
        });

        if (rolePage) {
            let needUpdate = false;
            if (!rolePage.path) {
                rolePage.path = '/system/role';
                needUpdate = true;
            }
            if (rolePage.parent_id !== systemMenuId) {
                rolePage.parent_id = systemMenuId;
                needUpdate = true;
            }
            if (rolePage.type !== 'page') {
                rolePage.type = 'page' as MenuType;
                needUpdate = true;
            }
            if (rolePage.status !== 'enabled') {
                rolePage.status = 'enabled' as MenuStatus;
                needUpdate = true;
            }
            if (rolePage.order !== SYSTEM_PAGE_ORDER.role) {
                rolePage.order = SYSTEM_PAGE_ORDER.role;
                needUpdate = true;
            }
            if (needUpdate) {
                await repo.menu.save(rolePage);
                logger.info(`Updated role page config: id=${rolePage.id}`);
            }
            return rolePage;
        }

        rolePage = await repo.menu.save(
            repo.menu.create({
                name: '角色管理',
                path: '/system/role',
                component: 'Role',
                type: 'page' as MenuType,
                status: 'enabled' as MenuStatus,
                parent_id: systemMenuId,
                order: SYSTEM_PAGE_ORDER.role,
            })
        );
        logger.info(`Created role page: id=${rolePage.id}`);

        return rolePage;
    }

    private async ensureRoleButtons(rolePageId: number) {
        const buttons = [
            { name: '查询角色', permission_code: 'system:role:query', order: 1 },
            { name: '新增角色', permission_code: 'system:role:add', order: 2 },
            { name: '编辑角色', permission_code: 'system:role:edit', order: 3 },
            { name: '删除角色', permission_code: 'system:role:delete', order: 4 },
            { name: '分配权限', permission_code: 'system:role:assign', order: 5 },
            { name: '查看权限', permission_code: 'system:role:perm', order: 6 },
        ];
        await this.ensureButtons(rolePageId, buttons);
    }

    private async ensureDeptPage(systemMenuId: number) {
        let deptPage = await repo.menu.findOne({
            where: { component: 'Dept' },
        });

        if (!deptPage) {
            deptPage = await repo.menu.findOne({
                where: { component: 'Department' },
            });
        }

        if (deptPage) {
            let needUpdate = false;
            if (deptPage.component !== 'Dept') {
                deptPage.component = 'Dept';
                needUpdate = true;
            }
            if (!deptPage.path) {
                deptPage.path = '/system/department';
                needUpdate = true;
            }
            if (deptPage.parent_id !== systemMenuId) {
                deptPage.parent_id = systemMenuId;
                needUpdate = true;
            }
            if (deptPage.type !== 'page') {
                deptPage.type = 'page' as MenuType;
                needUpdate = true;
            }
            if (deptPage.status !== 'enabled') {
                deptPage.status = 'enabled' as MenuStatus;
                needUpdate = true;
            }
            if (deptPage.order !== SYSTEM_PAGE_ORDER.department) {
                deptPage.order = SYSTEM_PAGE_ORDER.department;
                needUpdate = true;
            }
            if (needUpdate) {
                await repo.menu.save(deptPage);
                logger.info(`Updated department page config: id=${deptPage.id}`);
            }
            return deptPage;
        }

        deptPage = await repo.menu.save(
            repo.menu.create({
                name: '部门管理',
                path: '/system/department',
                component: 'Dept',
                type: 'page' as MenuType,
                status: 'enabled' as MenuStatus,
                parent_id: systemMenuId,
                order: SYSTEM_PAGE_ORDER.department,
            })
        );
        logger.info(`Created department page: id=${deptPage.id}`);

        return deptPage;
    }

    private async ensureDeptButtons(deptPageId: number) {
        const buttons = [
            { name: '查询部门', permission_code: 'system:dept:query', order: 1 },
            { name: '新增部门', permission_code: 'system:dept:add', order: 2 },
            { name: '编辑部门', permission_code: 'system:dept:edit', order: 3 },
            { name: '删除部门', permission_code: 'system:dept:delete', order: 4 },
        ];
        await this.ensureButtons(deptPageId, buttons);
    }

    private async ensureSuperAdminRole() {
        let superRole = await repo.role.findOne({
            where: { code: 'superadmin' },
            relations: ['menus'],
        });

        if (!superRole) {
            superRole = await repo.role.save(
                repo.role.create({
                    name: '超级管理员',
                    code: 'superadmin',
                    status: 'enabled' as RoleStatus,
                })
            );
            logger.info(`Created superadmin role: id=${superRole.id}`);
        }

        const allMenus = await repo.menu.find({
            where: { status: 'enabled' as MenuStatus },
        });
        const boundMenuIdSet = new Set((superRole.menus || []).map((menu) => menu.id));
        const shouldUpdateMenus =
            boundMenuIdSet.size !== allMenus.length ||
            allMenus.some((menu) => !boundMenuIdSet.has(menu.id));

        if (shouldUpdateMenus) {
            superRole.menus = allMenus;
            await repo.role.save(superRole);
            logger.info(
                `Bound all menus to superadmin role: roleId=${superRole.id}, menuCount=${allMenus.length}`
            );
        }

        return superRole;
    }

    private async ensureAdminUser(superRoleId: number) {
        let adminUser = await repo.user.findOne({
            where: { username: 'admin' },
            relations: ['roles'],
        });

        if (!adminUser) {
            const superRole = await repo.role.findOne({ where: { id: superRoleId } });
            adminUser = await repo.user.save(
                repo.user.create({
                    username: 'admin',
                    password_hash: await hashPassword('123456'),
                    nickname: '超级管理员',
                    status: 'enabled',
                    roles: superRole ? [superRole] : [],
                })
            );
            logger.info(`Created admin user: id=${adminUser.id}`);
            return;
        }

        const hasSuperRole = adminUser.roles?.some((r) => r.id === superRoleId);
        if (!hasSuperRole) {
            const superRole = await repo.role.findOne({ where: { id: superRoleId } });
            if (superRole) {
                adminUser.roles = [...(adminUser.roles || []), superRole];
                await repo.user.save(adminUser);
                logger.info(`Added superadmin role to existing admin user: id=${adminUser.id}`);
            }
        }
    }

    private validatePhone(phone?: string): void {
        if (phone && !PHONE_REGEX.test(phone)) {
            throwAppError(UserError.PHONE_INVALID);
        }
    }

    private validateEmail(email?: string): void {
        if (email && !EMAIL_REGEX.test(email)) {
            throwAppError(UserError.EMAIL_INVALID);
        }
    }

    private async checkPhoneUnique(phone?: string, excludeId?: number): Promise<void> {
        if (!phone) return;
        const where: FindOptionsWhere<User> = { phone };
        if (excludeId) where.id = Not(excludeId);
        const exist = await repo.user.findOne({ where });
        if (exist) throwAppError(UserError.PHONE_EXISTS);
    }

    private async checkEmailUnique(email?: string, excludeId?: number): Promise<void> {
        if (!email) return;
        const where: FindOptionsWhere<User> = { email };
        if (excludeId) where.id = Not(excludeId);
        const exist = await repo.user.findOne({ where });
        if (exist) throwAppError(UserError.EMAIL_EXISTS);
    }

    async list(params: UserQuery): Promise<QueryResult<UserItem>> {
        const pagination = normalizePagination(params);
        let qb = repo.user
            .createQueryBuilder('u')
            .leftJoinAndSelect('u.roles', 'r')
            .leftJoinAndSelect('u.department', 'd')
            .orderBy('u.id', 'ASC');

        qb = applyKeywordSearch(qb, pagination.keyword, [
            'u.username',
            'u.nickname',
            'u.phone',
            'u.email',
        ]);

        if (params.status) {
            qb = qb.andWhere('u.status = :status', { status: params.status });
        }

        if (params.department_id) {
            qb = qb.andWhere('u.department_id = :department_id', {
                department_id: params.department_id,
            });
        }

        return paginate(qb, pagination, this.toUserItem);
    }

    async listOptions(keyword?: string): Promise<UserOption[]> {
        const qb = repo.user
            .createQueryBuilder('u')
            .select(['u.id', 'u.username', 'u.nickname', 'u.phone', 'u.email'])
            .orderBy('u.id', 'ASC');

        if (keyword) {
            qb.andWhere(
                '(u.username LIKE :kw OR u.nickname LIKE :kw OR u.phone LIKE :kw OR u.email LIKE :kw)',
                {
                    kw: `%${keyword}%`,
                }
            );
        }

        const rows = await qb.getMany();
        return rows.map((row) => this.toUserOption(row));
    }

    async getById(id: number): Promise<UserItem> {
        const user = await repo.user.findOne({
            where: { id },
            relations: ['roles', 'department'],
        });
        if (!user) throwAppError(UserError.USER_NOT_FOUND);
        return this.toUserItem(user);
    }

    async create(data: CreateUserDto): Promise<void> {
        const exist = await repo.user.findOne({
            where: { username: data.username },
        });
        if (exist) throwAppError(UserError.USERNAME_EXISTS);

        this.validatePhone(data.phone);
        this.validateEmail(data.email);
        await this.checkPhoneUnique(data.phone);
        await this.checkEmailUnique(data.email);

        if (!data.role_ids?.length) {
            throwAppError(UserError.ROLE_REQUIRED);
        }
        const roles = await repo.role.findBy({ id: In(data.role_ids) });

        const user = repo.user.create({
            username: data.username,
            password_hash: await hashPassword(data.password),
            nickname: data.nickname,
            phone: data.phone,
            email: data.email,
            department_id: data.department_id ?? null,
            status: data.status ?? 'enabled',
            roles,
        });

        await repo.user.save(user);
    }

    async update(id: number, data: UpdateUserDto): Promise<void> {
        const user = await repo.user.findOne({
            where: { id },
            relations: ['roles'],
        });
        if (!user) throwAppError(UserError.USER_NOT_FOUND);
        if (user.username === 'admin' && data.role_ids !== undefined) {
            throwAppError(UserError.ADMIN_ROLE_PROTECTED);
        }
        if (data.phone !== undefined) {
            this.validatePhone(data.phone);
            await this.checkPhoneUnique(data.phone, id);
            user.phone = data.phone;
        }

        if (data.email !== undefined) {
            this.validateEmail(data.email);
            await this.checkEmailUnique(data.email, id);
            user.email = data.email;
        }

        if (data.nickname !== undefined) user.nickname = data.nickname;
        if (data.status !== undefined) user.status = data.status;
        if (data.department_id !== undefined) user.department_id = data.department_id;
        if (data.password) user.password_hash = await hashPassword(data.password);
        if (data.role_ids !== undefined) {
            if (data.role_ids.length === 0) {
                throwAppError(UserError.ROLE_REQUIRED);
            }
            user.roles = await repo.role.findBy({ id: In(data.role_ids) });
        }

        await repo.user.save(user);
        if (data.role_ids) {
            permissionService.clearUserCache(id);
        }
    }

    async remove(id: number): Promise<void> {
        const user = await repo.user.findOne({ where: { id } });
        if (user && user.username === 'admin') {
            throwAppError(UserError.ADMIN_DELETE_PROTECTED);
        }
        if (!user) throwAppError(UserError.USER_NOT_FOUND);
        await repo.user.softRemove(user);
        permissionService.clearUserCache(id);
    }
    async restore(id: number): Promise<void> {
        const result = await repo.user.restore(id);
        if (result.affected === 0) {
            throwAppError(UserError.USER_NOT_FOUND);
        }
    }
    async listDeleted(params: UserQuery): Promise<QueryResult<UserItem>> {
        const pagination = normalizePagination(params);
        const qb = repo.user
            .createQueryBuilder('u')
            .withDeleted()
            .where('u.deleted_at IS NOT NULL')
            .leftJoinAndSelect('u.roles', 'r')
            .leftJoinAndSelect('u.department', 'd')
            .orderBy('u.deleted_at', 'DESC');

        return paginate(qb, pagination, this.toUserItem);
    }
}

export const userService = new UserService();
