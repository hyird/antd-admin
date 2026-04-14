import type { Context } from 'hono';
import type { ZodSchema } from 'zod';
import { parseOrThrow } from './http';

export async function parseBody<T>(c: Context, schema: ZodSchema<T>): Promise<T> {
    return parseOrThrow(schema, await c.req.json());
}

export function parseParams<T>(c: Context, schema: ZodSchema<T>): T {
    return parseOrThrow(schema, c.req.param());
}

export function parseQuery<T>(schema: ZodSchema<T>, query: unknown): T {
    return parseOrThrow(schema, query);
}
