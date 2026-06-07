import type { Context } from 'hono';
import type { ZodSchema } from 'zod';
import { parseOrThrow } from './http.js';

export async function parseBody<T>(c: Context, schema: ZodSchema<T>): Promise<T> {
    return parseOrThrow(schema, await c.req.json());
}

export function parseParams<T>(c: Context, schema: ZodSchema<T>): T {
    return parseOrThrow(schema, c.req.param());
}

export function parseQuery<T>(schema: ZodSchema<T>, query: unknown): T {
    return parseOrThrow(schema, query);
}

export function getQuery(
    c: Context,
    key: string,
    options?: {
        aliases?: string[];
    }
): string | undefined {
    const keys = [key, ...(options?.aliases ?? [])];

    for (const currentKey of keys) {
        const value = c.req.query(currentKey);
        if (value !== undefined && value !== '') {
            return value;
        }
    }

    return undefined;
}

export function getQueryNumber(
    c: Context,
    key: string,
    options?: {
        aliases?: string[];
    }
): number | undefined {
    const value = getQuery(c, key, options);
    if (value === undefined) {
        return undefined;
    }

    const parsed = Number(value);
    return Number.isFinite(parsed) ? parsed : undefined;
}
