import { Column, Entity, Index, JoinColumn, ManyToOne } from 'typeorm';
import { BaseEntity } from '@/modules/common/types';
import { Device } from './device.entity';

/**
 * 变量状态枚举
 */
export type VariableStatus = 0 | 1;

/**
 * 变量实体
 *
 * 定义设备变量信息，供其他模块通过 TypeORM Repository 引用进行数据库读写。
 * 不提供通用 CRUD 接口，其他模块需要自行调用 repository 进行数据操作。
 *
 * @example
 * // 在其他模块中引用
 * import { Variable } from '@/modules/data/variable.entity';
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
