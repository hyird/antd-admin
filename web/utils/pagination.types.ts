export interface PageParams {
    page?: number | string;
    pageSize?: number | string;
    keyword?: string;
}

export interface PaginatedResponse<T> {
    list: T[];
    total: number;
    page?: number;
    page_size?: number;
    total_pages?: number;
}

export interface PaginatedResult<T> {
    list: T[];
    total: number;
    page?: number;
    pageSize?: number;
    totalPages?: number;
}
