/**
 * 统一环境变量加载
 */
import * as dotenv from 'dotenv';
import { existsSync } from 'fs';
import { join } from 'path';

const candidates = [join(process.cwd(), '.env')];

const envPath = candidates.find((p) => existsSync(p));
if (envPath) {
    dotenv.config({ path: envPath });
}
