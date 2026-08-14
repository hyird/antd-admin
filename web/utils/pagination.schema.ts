import { z } from 'zod';

export const pageParamsSchema = z.object({
    page: z
        .union([z.string(), z.number()])
        .optional()
        .transform((value) => {
            if (value === undefined || value === '') return 1;
            return Number(value);
        }),
    pageSize: z
        .union([z.string(), z.number()])
        .optional()
        .transform((value) => {
            if (value === undefined || value === '') return 10;
            return Number(value);
        }),
    keyword: z.string().optional(),
});
