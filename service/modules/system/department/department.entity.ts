import { Entity, Column, OneToMany } from 'typeorm';
import { BaseEntity } from '@/common/types';
import { User } from '@/modules/system/user/user.entity';
import type { DepartmentStatus } from './department.types';

export type { DepartmentStatus };

@Entity({ name: 'sys_department' })
export class Department extends BaseEntity {
    @Column({ name: 'name' })
    name!: string;

    @Column({ name: 'code', nullable: true })
    code?: string;

    @Column({ name: 'parent_id', type: 'int', nullable: true })
    parent_id?: number | null;

    @Column({ name: 'order', type: 'int', default: 0 })
    order!: number;

    @Column({ name: 'leader_id', type: 'int', nullable: true })
    leader_id?: number | null;

    @Column({ name: 'status', type: 'varchar', length: 20, default: 'enabled' })
    status!: DepartmentStatus;

    @OneToMany(() => User, (user) => user.department)
    users?: User[];
}
