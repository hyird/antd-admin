/**
 * 通用类型定义
 */
import type { SelectQueryBuilder } from 'typeorm';
import {
    PrimaryGeneratedColumn,
    CreateDateColumn,
    UpdateDateColumn,
    DeleteDateColumn,
} from 'typeorm';
import { z } from 'zod';

// ============ 实体基类 ============

export abstract class BaseEntity {
    @PrimaryGeneratedColumn()
    id!: number;

    @CreateDateColumn({ name: 'created_at', comment: '创建时间' })
    created_at!: Date;

    @UpdateDateColumn({ name: 'updated_at', comment: '更新时间' })
    updated_at!: Date;

    @DeleteDateColumn({ name: 'deleted_at', comment: '删除时间' })
    deleted_at?: Date | null;
}

// ============ 通用类型 ============

export interface ApiResponse<T = unknown> {
    code: number | string;
    message?: string;
    data?: T;
}

export interface ApiError {
    code: string | number;
    message: string;
    status?: number;
}

// 分页参数 (支持 string 类型，用于 HTTP 请求)
export interface PageParams {
    page?: number | string;
    pageSize?: number | string;
    page_size?: number | string;
    keyword?: string;
}

// 分页结果
export interface PageResult<T> {
    list: T[];
    total: number;
    page: number;
    pageSize: number;
    totalPages: number;
}

// 不分页结果（全部数据）
export interface ListResult<T> {
    list: T[];
    total: number;
}

// 统一列表结果类型（分页或不分页）
export type QueryResult<T> = PageResult<T> | ListResult<T>;

// 标准化后的分页参数
export interface NormalizedPagination {
    page: number;
    pageSize: number;
    skip: number;
    keyword?: string;
    /** 是否启用分页（page 和 pageSize 都不传时为 false） */
    paginated: boolean;
}

// Service 端的分页参数（支持 string 类型，因为来自 HTTP 请求）
export interface PaginationParams {
    page?: number | string;
    pageSize?: number | string;
    page_size?: number | string;
    keyword?: string;
}

export type { PageResult as PaginationResult };

// ============ Zod Schemas ============

export const pageParamsSchema = z.object({
    page: z
        .union([z.string(), z.number()])
        .optional()
        .transform((val) => {
            if (val === undefined || val === '') return 1;
            return Number(val);
        }),
    pageSize: z
        .union([z.string(), z.number()])
        .optional()
        .transform((val) => {
            if (val === undefined || val === '') return 10;
            return Number(val);
        }),
    page_size: z
        .union([z.string(), z.number()])
        .optional()
        .transform((val) => {
            if (val === undefined || val === '') return undefined;
            return Number(val);
        }),
    keyword: z.string().optional(),
});

export const idParamSchema = z.object({
    id: z.union([z.string(), z.number()]).transform((val) => Number(val)),
});

export type PageParamsInput = z.infer<typeof pageParamsSchema>;
export type IdParamInput = z.infer<typeof idParamSchema>;

// ============ 分页工具函数 ============

function hasPaginationParams(params: PaginationParams): boolean {
    const hasPage = params.page !== undefined && params.page !== null && params.page !== '';
    const resolvedPageSize = params.pageSize ?? params.page_size;
    const hasPageSize =
        resolvedPageSize !== undefined && resolvedPageSize !== null && resolvedPageSize !== '';
    return hasPage || hasPageSize;
}

export function normalizePagination(params: PaginationParams): NormalizedPagination {
    const paginated = hasPaginationParams(params);
    const page = Math.max(1, Number(params.page) || 1);
    const rawPageSize = params.pageSize ?? params.page_size;
    const pageSize = Math.min(100, Math.max(1, Number(rawPageSize) || 10));
    const keyword = params.keyword?.trim() || undefined;

    return {
        page,
        pageSize,
        skip: (page - 1) * pageSize,
        keyword,
        paginated,
    };
}

export async function paginate<Entity extends object, DTO>(
    qb: SelectQueryBuilder<Entity>,
    pagination: NormalizedPagination,
    transform: (entity: Entity) => DTO
): Promise<QueryResult<DTO>> {
    const { page, pageSize, skip, paginated } = pagination;

    if (!paginated) {
        const [rows, total] = await qb.getManyAndCount();
        return {
            list: rows.map(transform),
            total,
        };
    }

    const [rows, total] = await qb.skip(skip).take(pageSize).getManyAndCount();

    return {
        list: rows.map(transform),
        total,
        page,
        pageSize,
        totalPages: Math.ceil(total / pageSize),
    };
}

export function applyKeywordSearch<Entity extends object>(
    qb: SelectQueryBuilder<Entity>,
    keyword: string | undefined,
    fields: string[]
): SelectQueryBuilder<Entity> {
    if (!keyword || fields.length === 0) return qb;

    const conditions = fields.map((field) => `${field} LIKE :kw`).join(' OR ');
    return qb.andWhere(`(${conditions})`, { kw: `%${keyword}%` });
}
