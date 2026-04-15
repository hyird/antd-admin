import { Entity, Column, ManyToMany, JoinTable } from 'typeorm';
import { BaseEntity } from '@/common/types';
import { Menu } from '@/modules/system/menu/menu.entity';
import { User } from '@/modules/system/user/user.entity';
import type { RoleStatus } from './role.types';

export type { RoleStatus };

@Entity({ name: 'sys_role' })
export class Role extends BaseEntity {
    @Column({ name: 'code', unique: true })
    code!: string;

    @Column({ name: 'name' })
    name!: string;

    @Column({ name: 'description', nullable: true })
    description?: string;

    @Column({ name: 'status', type: 'varchar', length: 20, default: 'enabled' })
    status!: RoleStatus;

    @ManyToMany(() => Menu, (menu) => menu.roles, {
        onDelete: 'CASCADE',
    })
    @JoinTable({
        name: 'sys_role_menu',
        joinColumn: { name: 'role_id', referencedColumnName: 'id' },
        inverseJoinColumn: { name: 'menu_id', referencedColumnName: 'id' },
    })
    menus!: Menu[];

    @ManyToMany(() => User, (user) => user.roles)
    users!: User[];
}
