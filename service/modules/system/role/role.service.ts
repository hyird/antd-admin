import { repo } from '@/config/data';
import { In } from 'typeorm';
import { throwAppError } from '@/modules/common/http';
import { RoleError } from './role.error';
import {
    normalizePagination,
    paginate,
    applyKeywordSearch,
    type QueryResult,
} from '@/modules/common/types';
import type { RoleQuery, RoleItem, RoleDetail, CreateRoleDto, UpdateRoleDto } from './role.types';
import type { RoleStatus } from './role.entity';
import { Role } from './role.entity';

export class RoleService {
    private toRoleItem(r: Role): RoleItem {
        return {
            id: r.id,
            name: r.name,
            code: r.code,
            status: r.status,
            menu_ids: (r.menus || []).map((m) => m.id),
        };
    }

    private toRoleDetail(r: Role): RoleDetail {
        return {
            id: r.id,
            name: r.name,
            code: r.code,
            status: r.status,
            menu_ids: (r.menus || []).map((m) => m.id),
            menus: (r.menus || []).map((m) => ({
                id: m.id,
                name: m.name,
                type: m.type,
                parent_id: m.parent_id,
            })),
        };
    }

    async list(params: RoleQuery): Promise<QueryResult<RoleItem>> {
        const pagination = normalizePagination(params);
        let qb = repo.role
            .createQueryBuilder('r')
            .leftJoinAndSelect('r.menus', 'm')
            .orderBy('r.id', 'ASC');

        qb = applyKeywordSearch(qb, pagination.keyword, ['r.name', 'r.code']);

        if (params.status) {
            qb = qb.andWhere('r.status = :status', { status: params.status });
        }

        return paginate(qb, pagination, this.toRoleItem);
    }

    async getById(id: number): Promise<RoleDetail> {
        const role = await repo.role.findOne({
            where: { id },
            relations: ['menus'],
        });
        if (!role) {
            throwAppError(RoleError.NOT_FOUND);
        }
        return this.toRoleDetail(role);
    }

    async listAllEnabled() {
        const rows = await repo.role.find({
            where: { status: 'enabled' as RoleStatus },
            order: { id: 'ASC' },
        });
        return rows.map((r) => ({ id: r.id, name: r.name, code: r.code }));
    }

    async create(data: CreateRoleDto): Promise<void> {
        const exist = await repo.role.findOne({ where: { code: data.code } });
        if (exist) {
            throwAppError(RoleError.CODE_EXISTS);
        }

        const menus = data.menu_ids?.length
            ? await repo.menu.findBy({ id: In(data.menu_ids) })
            : [];

        const role = repo.role.create({
            name: data.name,
            code: data.code,
            status: (data.status ?? 'enabled') as RoleStatus,
            menus,
        });

        await repo.role.save(role);
    }

    async update(id: number, data: UpdateRoleDto): Promise<void> {
        const role = await repo.role.findOne({
            where: { id },
            relations: ['menus'],
        });
        if (!role) {
            throwAppError(RoleError.NOT_FOUND);
        }

        if (role.code === 'superadmin' && data.code && data.code !== 'superadmin') {
            throwAppError(RoleError.SUPERADMIN_CANNOT_MODIFY);
        }

        if (data.name !== undefined) role.name = data.name;

        if (data.code !== undefined && data.code !== role.code) {
            const exist = await repo.role.findOne({ where: { code: data.code } });
            if (exist && exist.id !== role.id) {
                throwAppError(RoleError.CODE_EXISTS);
            }
            role.code = data.code;
        }

        if (data.status !== undefined) role.status = data.status;
        if (data.menu_ids !== undefined) {
            role.menus = data.menu_ids.length
                ? await repo.menu.findBy({ id: In(data.menu_ids) })
                : [];
        }

        await repo.role.save(role);
    }

    async remove(id: number): Promise<void> {
        const role = await repo.role.findOne({ where: { id } });
        if (!role) throwAppError(RoleError.NOT_FOUND);
        if (role.code === 'superadmin') {
            throwAppError(RoleError.SUPERADMIN_CANNOT_DELETE);
        }
        const userCount = await repo.user
            .createQueryBuilder('u')
            .innerJoin('u.roles', 'r', 'r.id = :roleId', { roleId: id })
            .getCount();
        if (userCount > 0) throwAppError(RoleError.HAS_USERS);
        await repo.role.softRemove(role);
    }
}

export const roleService = new RoleService();
