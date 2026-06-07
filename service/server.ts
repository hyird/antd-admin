import 'reflect-metadata';
import './core/env.js';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';
import { existsSync, readFileSync } from 'node:fs';
import { Hono } from 'hono';
import { serve } from '@hono/node-server';
import { serveStatic } from '@hono/node-server/serve-static';
import { AppError, R } from './common/http.js';
import type { AppEnv } from './core/hono.env.js';
import { repo } from './config/data.js';
import { logger } from './utils/logger.js';
import { authRoute } from './modules/system/auth/auth.route.js';
import { roleRoute } from './modules/system/role/role.route.js';
import { menuRoute } from './modules/system/menu/menu.route.js';
import { userRoute } from './modules/system/user/user.route.js';
import { departmentRoute } from './modules/system/department/department.route.js';
import { userService } from './modules/system/user/user.service.js';

const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);
const webRoot = join(__dirname, 'web');
const serveWeb = existsSync(webRoot);
const indexHtml = serveWeb ? readFileSync(join(webRoot, 'index.html'), 'utf-8') : null;

const app = new Hono<AppEnv>();

app.onError((err, c) => {
    logger.error(`Unhandled error: ${err.message}`);

    if (err instanceof AppError) {
        return R.error(c, err);
    }

    return R.error(c, {
        code: 'INTERNAL_ERROR',
        message: '服务器内部错误',
        status: 500,
    });
});

app.get('/api/health', (c) => R.ok(c, { status: 'ok' }));
app.route('/api/auth', authRoute);
app.route('/api/users', userRoute);
app.route('/api/roles', roleRoute);
app.route('/api/menus', menuRoute);
app.route('/api/departments', departmentRoute);

if (serveWeb) {
    app.use('/*', serveStatic({ root: webRoot }));
    app.get('*', (c) => {
        return c.html(indexHtml || '');
    });
}

async function bootstrap() {
    try {
        await repo.initialize();
        await userService.seedAdmin();

        const port = Number(process.env.PORT || 1102);
        const hostname = process.env.HOST || '0.0.0.0';

        serve({
            fetch: app.fetch,
            port,
            hostname,
        });

        logger.info(`Server started on port ${port}`);
    } catch (error) {
        logger.error(`Initialization failed: ${error}`);
        process.exit(1);
    }
}

bootstrap();
