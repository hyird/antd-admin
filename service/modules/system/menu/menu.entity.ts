import { Entity, Column, Index, ManyToMany } from 'typeorm';
import { BaseEntity } from '@/common/types';
import { Role } from '@/modules/system/role/role.entity';
import type { MenuType, MenuStatus } from './menu.types';

export type { MenuType, MenuStatus };

@Entity({ name: 'sys_menu' })
@Index(['status'])
@Index(['type'])
@Index(['parent_id', 'type'])
export class Menu extends BaseEntity {
    @Column({ name: 'name' })
    name!: string;

    @Column({ name: 'path', nullable: true })
    path!: string | null;

    @Column({ name: 'icon', nullable: true })
    icon!: string | null;

    @Column({ name: 'parent_id', type: 'int', nullable: true })
    parent_id!: number | null;

    @Column({ name: 'order', type: 'int', default: 0 })
    order!: number;

    @Column({ name: 'type', type: 'varchar', length: 20, default: 'menu' })
    type!: MenuType;

    @Column({ name: 'component', nullable: true })
    component?: string;

    @Column({ name: 'status', type: 'varchar', length: 20, default: 'enabled' })
    status!: MenuStatus;

    @Column({ name: 'permission_code', nullable: true })
    permission_code!: string;

    @Column({ name: 'is_default', type: 'boolean', default: false })
    is_default!: boolean;

    @ManyToMany(() => Role, (role) => role.menus, {
        onDelete: 'CASCADE',
    })
    roles!: Role[];
}
