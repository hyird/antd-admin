import { Entity, Column, Index, ManyToMany, ManyToOne, JoinTable } from 'typeorm';
import { Role } from '@/modules/system/role/role.entity';
import { Department } from '@/modules/system/department/department.entity';
import { BaseEntity } from '@/modules/common/types';
import type { UserStatus } from './user.types';

export type { UserStatus };

@Entity({ name: 'sys_user' })
@Index(['status'])
@Index(['department_id'])
export class User extends BaseEntity {
    @Column({ name: 'username', unique: true })
    username!: string;

    @Column({ name: 'password_hash' })
    password_hash!: string;

    @Column({ name: 'nickname', nullable: true })
    nickname?: string;

    @Column({ name: 'phone', nullable: true })
    phone?: string;

    @Column({ name: 'email', nullable: true })
    email?: string;

    @Column({ name: 'department_id', type: 'int', nullable: true })
    department_id?: number | null;

    @ManyToOne(() => Department, (dept) => dept.users, { nullable: true })
    department?: Department | null;

    @Column({ name: 'status', type: 'varchar', length: 20, default: 'enabled' })
    status!: UserStatus;

    @ManyToMany(() => Role, (role) => role.users)
    @JoinTable({
        name: 'sys_user_role',
        joinColumn: { name: 'user_id', referencedColumnName: 'id' },
        inverseJoinColumn: { name: 'role_id', referencedColumnName: 'id' },
    })
    roles!: Role[];
}
