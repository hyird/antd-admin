import { repo } from '@/config/data';
import { throwAppError } from '@/modules/common/http';
import { DepartmentError } from './department.error';
import type {
    DepartmentItem,
    DepartmentTreeItem,
    CreateDepartmentDto,
    UpdateDepartmentDto,
} from './department.types';
import type { DepartmentStatus } from './department.entity';
import { Department } from './department.entity';
import { Not } from 'typeorm';
import { buildTree } from '@/utils/tree';

export class DepartmentService {
    private toFlatItem(d: Department): DepartmentItem {
        return {
            id: d.id,
            name: d.name,
            code: d.code,
            parent_id: d.parent_id ?? undefined,
            sort_order: d.order,
            leader_id: d.leader_id ?? undefined,
            status: d.status,
        };
    }

    private getAllChildIds(parentId: number, childMap: Map<number, number[]>): number[] {
        const children = childMap.get(parentId) ?? [];
        const ids: number[] = [];
        for (const childId of children) {
            ids.push(childId);
            const grandChildren = this.getAllChildIds(childId, childMap);
            ids.push(...grandChildren);
        }
        return ids;
    }

    async listAll(keyword?: string): Promise<DepartmentItem[]> {
        const qb = repo.department
            .createQueryBuilder('d')
            .orderBy('d.order', 'ASC')
            .addOrderBy('d.id', 'ASC');

        if (keyword) {
            qb.where('d.name LIKE :kw OR d.code LIKE :kw', { kw: `%${keyword}%` });
        }

        const rows = await qb.getMany();
        return rows.map((d) => this.toFlatItem(d));
    }

    async getTree(status?: DepartmentStatus): Promise<DepartmentTreeItem[]> {
        const qb = repo.department
            .createQueryBuilder('d')
            .orderBy('d.order', 'ASC')
            .addOrderBy('d.id', 'ASC');

        if (status) {
            qb.where('d.status = :status', { status });
        }

        const rows = await qb.getMany();
        return buildTree(
            rows.map((d) => this.toFlatItem(d)),
            { sortBy: 'order' }
        ) as DepartmentTreeItem[];
    }

    async getById(id: number): Promise<DepartmentItem> {
        const dept = await repo.department.findOne({ where: { id } });
        if (!dept) throwAppError(DepartmentError.NOT_FOUND);
        return this.toFlatItem(dept);
    }

    async create(data: CreateDepartmentDto): Promise<void> {
        if (data.code) {
            const exist = await repo.department.findOne({
                where: { code: data.code },
            });
            if (exist) throwAppError(DepartmentError.CODE_EXISTS);
        }

        if (data.parent_id) {
            const parent = await repo.department.findOne({
                where: { id: data.parent_id },
            });
            if (!parent) throwAppError(DepartmentError.NOT_FOUND);
        }

        const dept = repo.department.create({
            name: data.name,
            code: data.code,
            parent_id: data.parent_id ?? null,
            order: data.sort_order ?? 0,
            leader_id: data.leader_id ?? null,
            status: (data.status ?? 'enabled') as DepartmentStatus,
        });

        await repo.department.save(dept);
    }

    async update(id: number, data: UpdateDepartmentDto): Promise<void> {
        const dept = await repo.department.findOne({ where: { id } });
        if (!dept) throwAppError(DepartmentError.NOT_FOUND);

        if (data.code !== undefined && data.code !== dept.code) {
            const exist = await repo.department.findOne({
                where: { code: data.code, id: Not(id) },
            });
            if (exist) throwAppError(DepartmentError.CODE_EXISTS);
        }

        if (data.parent_id !== undefined) {
            if (data.parent_id === id) {
                throwAppError(DepartmentError.PARENT_SELF);
            }

            if (data.parent_id) {
                const parent = await repo.department.findOne({
                    where: { id: data.parent_id },
                });
                if (!parent) throwAppError(DepartmentError.NOT_FOUND);

                const relationRows = await repo.department.find({
                    select: ['id', 'parent_id'],
                });
                const childMap = new Map<number, number[]>();
                for (const row of relationRows) {
                    if (!row.parent_id) continue;
                    const children = childMap.get(row.parent_id) ?? [];
                    children.push(row.id);
                    childMap.set(row.parent_id, children);
                }

                const childIds = this.getAllChildIds(id, childMap);
                if (childIds.includes(data.parent_id)) {
                    throwAppError(DepartmentError.PARENT_IS_CHILD);
                }
            }

            dept.parent_id = data.parent_id;
        }

        if (data.name !== undefined) dept.name = data.name;
        if (data.code !== undefined) dept.code = data.code;
        if (data.sort_order !== undefined) dept.order = data.sort_order;
        if (data.leader_id !== undefined) dept.leader_id = data.leader_id;
        if (data.status !== undefined) dept.status = data.status;

        await repo.department.save(dept);
    }

    async remove(id: number): Promise<void> {
        const dept = await repo.department.findOne({ where: { id } });
        if (!dept) throwAppError(DepartmentError.NOT_FOUND);
        const childCount = await repo.department.count({
            where: { parent_id: id },
        });
        if (childCount > 0) throwAppError(DepartmentError.HAS_CHILDREN);
        const userCount = await repo.user.count({ where: { department_id: id } });
        if (userCount > 0) throwAppError(DepartmentError.HAS_USERS);
        await repo.department.softRemove(dept);
    }
}

export const departmentService = new DepartmentService();
