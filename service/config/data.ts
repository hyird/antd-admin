import 'reflect-metadata';
import '@/core/env';
import { DataSource, Repository } from 'typeorm';
import { createLogger } from '@/utils/logger';
import { User } from '@/modules/system/user/user.entity';
import { Role } from '@/modules/system/role/role.entity';
import { Menu } from '@/modules/system/menu/menu.entity';
import { Department } from '@/modules/system/department/department.entity';
import { Device } from '@/modules/data/device.entity';
import { Variable } from '@/modules/data/variable.entity';
import { VariableHistory } from '@/modules/data/variable-history.entity';
import { VariableRealtime } from '@/modules/data/variable-realtime.entity';

const isProduction = process.env.NODE_ENV === 'production';
const shouldSynchronize =
    process.env.DB_SYNCHRONIZE !== undefined
        ? process.env.DB_SYNCHRONIZE === 'true'
        : !isProduction;

export const AppDataSource = new DataSource({
    type: 'mariadb',
    host: process.env.DB_HOST,
    port: Number(process.env.DB_PORT),
    username: process.env.DB_USERNAME,
    password: process.env.DB_PASSWORD,
    database: process.env.DB_DATABASE,
    charset: 'utf8mb4_general_ci',
    synchronize: shouldSynchronize,
    entities: [User, Role, Menu, Department, Device, Variable, VariableHistory, VariableRealtime],
    poolSize: 10,
    extra: {
        connectionLimit: 10,
    },
});

const logger = createLogger('repo');

class RepositoryManager {
    private initialized = false;

    private ensureInitialized(): void {
        if (!this.initialized) {
            throw new Error('RepositoryManager not initialized. Call initialize() first.');
        }
    }

    async initialize(): Promise<void> {
        if (this.initialized) return;

        await AppDataSource.initialize();
        this.initialized = true;
        logger.info('DataSource initialized');
    }

    get user(): Repository<User> {
        this.ensureInitialized();
        return AppDataSource.getRepository(User);
    }

    get role(): Repository<Role> {
        this.ensureInitialized();
        return AppDataSource.getRepository(Role);
    }

    get menu(): Repository<Menu> {
        this.ensureInitialized();
        return AppDataSource.getRepository(Menu);
    }

    get department(): Repository<Department> {
        this.ensureInitialized();
        return AppDataSource.getRepository(Department);
    }

    get device(): Repository<Device> {
        this.ensureInitialized();
        return AppDataSource.getRepository(Device);
    }

    get variable(): Repository<Variable> {
        this.ensureInitialized();
        return AppDataSource.getRepository(Variable);
    }

    get variableHistory(): Repository<VariableHistory> {
        this.ensureInitialized();
        return AppDataSource.getRepository(VariableHistory);
    }

    get variableRealtime(): Repository<VariableRealtime> {
        this.ensureInitialized();
        return AppDataSource.getRepository(VariableRealtime);
    }
}

export const repo = new RepositoryManager();
