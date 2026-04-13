/**
 * HTTP 响应与错误处理
 */
import type { Context } from 'hono';
import type { ContentfulStatusCode } from 'hono/utils/http-status';
import type { QueryResult } from './types';

export interface ApiResponse<T = unknown> {
    code: number | string;
    message: string;
    data?: T;
}

export interface ApiPageResponse<T> extends ApiResponse<QueryResult<T>> {}

export interface AppErrorDef {
    code: string | number;
    message: string;
    status: ContentfulStatusCode;
}

export class AppError extends Error {
    status: ContentfulStatusCode;
    code: string | number;

    constructor(code: string | number, message: string, status: ContentfulStatusCode = 400) {
        super(message);
        this.code = code;
        this.status = status;
    }
}

export function throwAppError(err: AppErrorDef): never {
    throw new AppError(err.code, err.message, err.status);
}

class ResponseBuilder {
    ok<T>(c: Context, data?: T, message = 'ok'): Response {
        return c.json<ApiResponse<T>>({ code: 0, message, data });
    }

    page<T>(c: Context, data: QueryResult<T>, message = 'ok'): Response {
        return c.json<ApiPageResponse<T>>({ code: 0, message, data });
    }

    created<T = void>(c: Context, data?: T, message = '创建成功'): Response {
        return c.json<ApiResponse<T>>({ code: 0, message, data }, 201);
    }

    updated(c: Context, message = '更新成功'): Response {
        return c.json<ApiResponse>({ code: 0, message });
    }

    deleted(c: Context, message = '删除成功'): Response {
        return c.json<ApiResponse>({ code: 0, message });
    }

    error(c: Context, err: AppErrorDef): Response {
        return c.json<ApiResponse>({ code: err.code, message: err.message }, err.status);
    }
}

export const R = new ResponseBuilder();
