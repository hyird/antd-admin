import { Column, Entity, Index } from 'typeorm';
import { BaseEntity } from '@/modules/common/types';

/**
 * 设备状态枚举
 */
export type DeviceStatus = 'enabled' | 'disabled';

/**
 * 设备实体
 *
 * 定义设备基础信息，供其他模块通过 TypeORM Repository 引用进行数据库读写。
 * 不提供通用 CRUD 接口，其他模块需要自行调用 repository 进行数据操作。
 *
 * @example
 * // 在其他模块中引用
 * import { Device } from '@/modules/data/device.entity';
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
