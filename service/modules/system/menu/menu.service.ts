import { repo } from '@/config/data';
import { throwAppError } from '@/modules/common/http';
import { MenuError } from './menu.error';
import { buildMenuTree } from '@/utils/tree';
import type {
    MenuItem,
    MenuTreeItem,
    CreateMenuDto,
    UpdateMenuDto,
    ReorderMenuDto,
    BatchCreateMenuButtonsDto,
} from './menu.types';
import type { MenuStatus, MenuType } from './menu.entity';
import { Menu } from './menu.entity';
import { In } from 'typeorm';

function checkParentChildType(parentType: MenuType | null, childType: MenuType): void {
    if (!parentType) return;
    if (parentType === 'button') {
        throwAppError(MenuError.MENU_TYPE_INVALID);
    }
    if (parentType === 'page' && childType !== 'button') {
        throwAppError(MenuError.MENU_TYPE_INVALID);
    }
}

export class MenuService {
    private toFlatItem(m: Menu): MenuItem {
        return {
            id: m.id,
            name: m.name,
            path: m.path ?? undefined,
            component: m.component ?? undefined,
            icon: m.icon ?? undefined,
            parent_id: m.parent_id ?? undefined,
            sort_order: m.order,
            type: m.type as MenuType,
            status: m.status as MenuStatus,
            permission_code: m.permission_code ?? undefined,
            full_path: m.path ?? undefined,
        };
    }

    async listAll(keyword?: string): Promise<MenuItem[]> {
        const qb = repo.menu
            .createQueryBuilder('m')
            .orderBy('m.order', 'ASC')
            .addOrderBy('m.id', 'ASC');

        if (keyword) {
            qb.where('m.name LIKE :kw OR m.path LIKE :kw', { kw: `%${keyword}%` });
        }

        const rows = await qb.getMany();
        return rows.map((m) => this.toFlatItem(m));
    }

    async getTree(status?: MenuStatus): Promise<MenuTreeItem[]> {
        const qb = repo.menu
            .createQueryBuilder('m')
            .orderBy('m.order', 'ASC')
            .addOrderBy('m.id', 'ASC');

        if (status) {
            qb.where('m.status = :status', { status });
        }

        const rows = await qb.getMany();
        // 菜单管理页面需要显示按钮权限，所以不过滤 button
        return buildMenuTree(
            rows.map((m) => this.toFlatItem(m)),
            false
        );
    }

    async getDetail(id: number): Promise<MenuItem> {
        const menu = await repo.menu.findOne({ where: { id } });
        if (!menu) {
            throwAppError(MenuError.MENU_NOT_FOUND);
        }
        return this.toFlatItem(menu);
    }

    async create(data: CreateMenuDto): Promise<void> {
        const type: MenuType = (data.type ?? 'menu') as MenuType;
        const normalizedIcon = data.icon?.trim();
        let parentType: MenuType | null = null;

        if (data.parent_id) {
            const parent = await repo.menu.findOne({ where: { id: data.parent_id } });
            if (!parent) {
                throwAppError(MenuError.MENU_PARENT_NOT_FOUND);
            }
            parentType = parent.type;
        }

        checkParentChildType(parentType, type);
        if (data.is_default && type !== 'page') {
            throwAppError(MenuError.DEFAULT_MUST_BE_PAGE);
        }
        const menu = repo.menu.create({
            name: data.name,
            path: data.path,
            component: data.component,
            icon: normalizedIcon || null,
            parent_id: data.parent_id ?? null,
            order: data.sort_order ?? 0,
            type,
            status: (data.status ?? 'enabled') as MenuStatus,
            permission_code: data.permission_code,
            is_default: data.is_default ?? false,
        });
        if (data.is_default) {
            await repo.menu.update({ is_default: true }, { is_default: false });
        }
        await repo.menu.save(menu);
    }

    async update(id: number, data: UpdateMenuDto): Promise<void> {
        const menu = await repo.menu.findOne({ where: { id } });
        if (!menu) {
            throwAppError(MenuError.MENU_NOT_FOUND);
        }

        const newType: MenuType = (data.type ?? menu.type) as MenuType;
        const newParentId = data.parent_id === undefined ? menu.parent_id : data.parent_id;

        let parentType: MenuType | null = null;
        if (newParentId) {
            const parent = await repo.menu.findOne({ where: { id: newParentId } });
            if (!parent) {
                throwAppError(MenuError.MENU_PARENT_NOT_FOUND);
            }
            parentType = parent.type;
        }

        checkParentChildType(parentType, newType);

        if (data.name !== undefined) menu.name = data.name;
        if (data.path !== undefined) menu.path = data.path;
        if (data.icon !== undefined) {
            const normalizedIcon = data.icon.trim();
            menu.icon = normalizedIcon || null;
        }
        if (data.component !== undefined) menu.component = data.component;
        if (data.sort_order !== undefined) menu.order = data.sort_order;
        if (data.type !== undefined) menu.type = data.type;
        if (data.status !== undefined) menu.status = data.status;
        if (data.permission_code !== undefined) menu.permission_code = data.permission_code;
        if (data.parent_id !== undefined) {
            if (data.parent_id === null) {
                menu.parent_id = null;
            } else if (data.parent_id === id) {
                throwAppError(MenuError.MENU_PARENT_SELF);
            } else {
                menu.parent_id = data.parent_id;
            }
        }
        if (data.is_default && data.type !== 'page') {
            throwAppError(MenuError.DEFAULT_MUST_BE_PAGE);
        }
        if (data.is_default !== undefined) {
            if (data.is_default) {
                await repo.menu.update({ is_default: true }, { is_default: false });
            }
            menu.is_default = data.is_default;
        }
        await repo.menu.save(menu);
    }

    async remove(id: number): Promise<void> {
        const menu = await repo.menu.findOne({ where: { id } });
        if (!menu) throwAppError(MenuError.MENU_NOT_FOUND);
        const childCount = await repo.menu.count({ where: { parent_id: id } });
        if (childCount > 0) throwAppError(MenuError.MENU_HAS_CHILDREN);
        await repo.menu.softRemove(menu);
    }

    async reorder(data: ReorderMenuDto): Promise<void> {
        const items = data.items ?? [];
        if (items.length === 0) {
            return;
        }

        const ids = items.map((item) => item.id);
        const menus = await repo.menu.find({ where: { id: In(ids) } });
        if (menus.length !== ids.length) {
            throwAppError(MenuError.MENU_NOT_FOUND);
        }

        const menuMap = new Map(menus.map((menu) => [menu.id, menu]));

        await repo.menu.manager.transaction(async (manager) => {
            const txRepo = manager.getRepository(Menu);
            const updatedMenus: Menu[] = [];
            for (const item of items) {
                const menu = menuMap.get(item.id);
                if (!menu) {
                    throwAppError(MenuError.MENU_NOT_FOUND);
                }
                if ((item.parent_id ?? null) !== (menu.parent_id ?? null)) {
                    throwAppError(MenuError.MENU_TYPE_INVALID);
                }
                menu.order = item.sort_order;
                updatedMenus.push(menu);
            }
            await txRepo.save(updatedMenus);
        });
    }

    async batchCreateButtons(data: BatchCreateMenuButtonsDto): Promise<number> {
        const parent = await repo.menu.findOne({ where: { id: data.parent_id } });
        if (!parent) {
            throwAppError(MenuError.MENU_PARENT_NOT_FOUND);
        }
        if (parent.type !== 'page') {
            throwAppError(MenuError.MENU_TYPE_INVALID);
        }

        const normalizedItems = (data.items ?? [])
            .map((item) => ({
                name: item.name?.trim(),
                permission_code: item.permission_code?.trim(),
            }))
            .filter((item) => item.name && item.permission_code) as Array<{
            name: string;
            permission_code: string;
        }>;

        if (normalizedItems.length === 0) {
            return 0;
        }

        const uniqueItems = Array.from(
            new Map(normalizedItems.map((item) => [item.permission_code, item])).values()
        );
        const permissionCodes = uniqueItems.map((item) => item.permission_code);

        const existingButtons = await repo.menu.find({
            where: {
                parent_id: data.parent_id,
                type: 'button',
                permission_code: In(permissionCodes),
            },
            select: { permission_code: true },
        });

        const existingCodeSet = new Set(
            existingButtons
                .map((button) => button.permission_code)
                .filter((code): code is string => Boolean(code))
        );

        const toCreateItems = uniqueItems.filter(
            (item) => !existingCodeSet.has(item.permission_code)
        );
        if (toCreateItems.length === 0) {
            return 0;
        }

        const maxOrderRow = await repo.menu
            .createQueryBuilder('m')
            .select('COALESCE(MAX(m.order), 0)', 'maxOrder')
            .where('m.parent_id = :parent_id', { parent_id: data.parent_id })
            .getRawOne<{ maxOrder: string | number }>();
        const maxOrder = Number(maxOrderRow?.maxOrder ?? 0) || 0;

        const createdButtons = toCreateItems.map((item, index) =>
            repo.menu.create({
                name: item.name,
                parent_id: data.parent_id,
                type: 'button',
                status: 'enabled',
                permission_code: item.permission_code,
                order: maxOrder + index + 1,
                path: null,
                component: undefined,
                icon: null,
            })
        );

        await repo.menu.save(createdButtons);
        return createdButtons.length;
    }
}

export const menuService = new MenuService();
