import { Column, Entity, Index } from 'typeorm';
import { BaseEntity } from '@/modules/common/types';

/**
 * 变量历史数据实体
 *
 * 存储变量的历史采集记录。
 * 供其他模块通过 TypeORM Repository 引用进行数据库读写。
 * 不提供通用 CRUD 接口，其他模块需要自行调用 repository 进行数据操作。
 *
 * @example
 * // 在其他模块中引用
 * import { VariableHistory } from '@/modules/data/variable-history.entity';
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
