import {
    Column,
    Entity,
    Index,
    JoinColumn,
    ManyToOne,
    PrimaryColumn,
    UpdateDateColumn,
} from 'typeorm';
import { BaseEntity } from '../../common/types.js';

/**
 * 设备状态枚举
 */
export type DeviceStatus = 'enabled' | 'disabled';

/**
 * 变量状态枚举
 */
export type VariableStatus = 0 | 1;

/**
 * 设备实体
 *
 * 定义设备基础信息，供其他模块通过 TypeORM Repository 引用进行数据库读写。
 * 不提供通用 CRUD 接口，其他模块需要自行调用 repository 进行数据操作。
 *
 * @example
 * // 在其他模块中引用
 * import { Device } from './data.entity.js';
 *
 * const deviceRepo = dataSource.getRepository(Device);
 * const devices = await deviceRepo.find();
 */
@Entity({ name: 'sys_device' })
@Index(['code'], { unique: true })
@Index(['name'])
@Index(['status'])
export class Device extends BaseEntity {
    @Column({ name: 'code', length: 100, comment: '设备编码' })
    code!: string;

    @Column({ name: 'name', length: 200, comment: '设备名称' })
    name!: string;

    @Column({
        name: 'data_source',
        length: 100,
        nullable: true,
        comment: '数据来源',
    })
    data_source?: string;

    @Column({
        name: 'device_type',
        length: 100,
        nullable: true,
        comment: '设备类型',
    })
    device_type?: string;

    @Column({
        name: 'facility_type',
        length: 100,
        nullable: true,
        comment: '监测设施',
    })
    facility_type?: string;

    @Column({
        name: 'monitoring_type',
        length: 100,
        nullable: true,
        comment: '监测类型',
    })
    monitoring_type?: string;

    @Column({
        name: 'status',
        type: 'varchar',
        length: 20,
        default: 'enabled',
        comment: '状态',
    })
    status!: DeviceStatus;

    @Column({ name: 'remark', length: 500, nullable: true, comment: '备注' })
    remark?: string;
}

/**
 * 变量实体
 *
 * 定义设备变量信息，供其他模块通过 TypeORM Repository 引用进行数据库读写。
 * 不提供通用 CRUD 接口，其他模块需要自行调用 repository 进行数据操作。
 *
 * @example
 * // 在其他模块中引用
 * import { Variable } from './data.entity.js';
 *
 * const variableRepo = dataSource.getRepository(Variable);
 * const variables = await variableRepo.find();
 */
@Entity({ name: 'sys_variable' })
@Index(['code'], { unique: true })
@Index(['device_id'])
@Index(['device_id', 'sort_order'])
@Index(['enabled'])
export class Variable extends BaseEntity {
    @Column({ name: 'device_id', type: 'int', comment: '设备ID' })
    device_id!: number;

    @ManyToOne(() => Device, { onDelete: 'RESTRICT' })
    @JoinColumn({ name: 'device_id' })
    device?: Device;

    @Column({ name: 'name', length: 200, comment: '变量名称' })
    name!: string;

    @Column({ name: 'code', length: 100, comment: '变量唯一编码' })
    code!: string;

    @Column({ name: 'unit', length: 50, nullable: true, comment: '单位' })
    unit?: string;

    @Column({ name: 'category', length: 100, nullable: true, comment: '分类' })
    category?: string;

    @Column({ name: 'description', length: 500, nullable: true, comment: '描述' })
    description?: string;

    @Column({
        name: 'scale_factor',
        type: 'decimal',
        precision: 12,
        scale: 6,
        default: 1,
        comment: '缩放系数',
    })
    scale_factor!: number;

    @Column({
        name: 'enabled',
        type: 'tinyint',
        default: 1,
        comment: '是否启用 (1=启用, 0=禁用)',
    })
    enabled!: VariableStatus;

    @Column({ name: 'sort_order', type: 'int', default: 0, comment: '排序' })
    sort_order!: number;
}

/**
 * 变量历史数据实体
 *
 * 存储变量的历史采集记录。
 * 供其他模块通过 TypeORM Repository 引用进行数据库读写。
 * 不提供通用 CRUD 接口，其他模块需要自行调用 repository 进行数据操作。
 *
 * @example
 * // 在其他模块中引用
 * import { VariableHistory } from './data.entity.js';
 *
 * const historyRepo = dataSource.getRepository(VariableHistory);
 * const history = await historyRepo.find({ where: { code: 'xxx' } });
 */
@Entity({ name: 'sys_variable_history' })
@Index(['code', 'collect_time'], { unique: true })
@Index(['device_id'])
@Index(['collect_time'])
export class VariableHistory extends BaseEntity {
    @Column({ name: 'device_id', type: 'int', comment: '设备ID' })
    device_id!: number;

    @Column({ name: 'device_code', length: 100, comment: '设备编码' })
    device_code!: string;

    @Column({ name: 'device_name', length: 200, comment: '设备名称' })
    device_name!: string;

    @Column({ name: 'variable_name', length: 200, comment: '变量名称' })
    variable_name!: string;

    @Column({ name: 'code', length: 100, comment: '变量编码' })
    code!: string;

    @Column({
        name: 'value',
        type: 'decimal',
        precision: 12,
        scale: 4,
        nullable: true,
        comment: '数值',
    })
    value!: number | null;

    @Column({ name: 'collect_time', type: 'datetime', comment: '采集时间' })
    collect_time!: Date;
}

/**
 * 变量实时数据实体
 *
 * 存储变量的最新采集值，以变量编码为主键。
 * 供其他模块通过 TypeORM Repository 引用进行数据库读写。
 * 不提供通用 CRUD 接口，其他模块需要自行调用 repository 进行数据操作。
 *
 * @example
 * // 在其他模块中引用
 * import { VariableRealtime } from './data.entity.js';
 *
 * const realtimeRepo = dataSource.getRepository(VariableRealtime);
 * const latestValue = await realtimeRepo.findOne({ where: { code: 'xxx' } });
 */
@Entity({ name: 'sys_variable_realtime' })
@Index(['device_id'])
@Index(['collect_time'])
export class VariableRealtime {
    @PrimaryColumn({ name: 'code', length: 100, comment: '变量编码' })
    code!: string;

    @Column({ name: 'device_id', type: 'int', comment: '设备ID' })
    device_id!: number;

    @Column({ name: 'device_code', length: 100, comment: '设备编码' })
    device_code!: string;

    @Column({ name: 'device_name', length: 200, comment: '设备名称' })
    device_name!: string;

    @Column({ name: 'variable_name', length: 200, comment: '变量名称' })
    variable_name!: string;

    @Column({
        name: 'value',
        type: 'decimal',
        precision: 12,
        scale: 4,
        nullable: true,
        comment: '数值',
    })
    value!: number | null;

    @Column({
        name: 'collect_time',
        type: 'datetime',
        nullable: true,
        comment: '采集时间',
    })
    collect_time!: Date | null;

    @UpdateDateColumn({
        name: 'updated_at',
        type: 'datetime',
        comment: '更新时间',
    })
    updated_at!: Date;
}
