/**
 * 角色管理
 */

export type {
    Role,
    RoleStatus,
    RoleItem,
    RoleDetail,
    RoleOption,
    RoleQuery,
    CreateRoleDto,
    UpdateRoleDto,
} from './role.types';
export { roleKeys, roleQueryKeys } from './role.types';

export {
    useRoleList,
    useRoleDetail,
    useRoleOptions,
    useRoleUpdate,
    useRoleDelete,
    useRoleSave,
} from './role.service';
export { getList, getDetail, getAll, create, update, remove } from './role.api';

import type { TreeDataNode, TreeProps } from 'antd';
import {
    App,
    Button,
    Form,
    Input,
    Modal,
    Pagination,
    Result,
    Select,
    Space,
    Table,
    Tag,
    Tree,
} from 'antd';
import type { ColumnsType } from 'antd/es/table';
import { useMemo, useState } from 'react';
import { PageContainer } from '@/components/PageContainer';
import { StatusTag } from '@/components/StatusTag';
import { useDebounceFn } from '@/hooks/useDebounceFn';
import { usePermissions } from '@/hooks/usePermission';
import { useMenuTree } from '../menu/menu.service';
import {
    useRoleDelete,
    useRoleDetail,
    useRoleList,
    useRoleSave,
    useRoleUpdate,
} from './role.service';
import type { Menu } from '../menu/menu.types';
import type { Role } from './role.types';

const { Search } = Input;

interface RoleFormValues {
    id?: number;
    name: string;
    code: string;
    status: Role.Status;
    menu_ids: number[];
}

function menuTreeToTreeData(menus: Menu.TreeItem[]): TreeDataNode[] {
    return menus.map((m) => ({
        key: m.id,
        title: `${m.name}${m.type === 'button' ? ' [按钮]' : m.type === 'page' ? ' [页面]' : ''}`,
        children: m.children?.length ? menuTreeToTreeData(m.children) : undefined,
    }));
}

function getAllTreeKeys(menus: Menu.TreeItem[]): number[] {
    const keys: number[] = [];
    const traverse = (nodes: Menu.TreeItem[]) => {
        nodes.forEach((n) => {
            keys.push(n.id);
            if (n.children?.length) traverse(n.children);
        });
    };
    traverse(menus);
    return keys;
}

function getParentKeys(menus: Menu.TreeItem[]): Set<number> {
    const parentKeys = new Set<number>();
    const traverse = (nodes: Menu.TreeItem[]) => {
        nodes.forEach((n) => {
            if (n.children?.length) {
                parentKeys.add(n.id);
                traverse(n.children);
            }
        });
    };
    traverse(menus);
    return parentKeys;
}

const SystemRolePage = () => {
    const [keyword, setKeyword] = useState('');
    const [pagination, setPagination] = useState({ page: 1, pageSize: 10 });
    const [modalVisible, setModalVisible] = useState(false);
    const [permModalVisible, setPermModalVisible] = useState(false);
    const [editing, setEditing] = useState<Role.Item | null>(null);
    const [currentRole, setCurrentRole] = useState<Role.Item | null>(null);
    const [userCheckedKeys, setUserCheckedKeys] = useState<number[] | null>(null);
    const [userExpandedKeys, setUserExpandedKeys] = useState<number[] | null>(null);

    const [form] = Form.useForm<RoleFormValues>();
    const { modal } = App.useApp();

    const { has } = usePermissions();
    const canQuery = has('system:role:query');
    const canAdd = has('system:role:add');
    const canEdit = has('system:role:edit');
    const canDelete = has('system:role:delete');
    const canPerm = has('system:role:perm');

    const doSearch = (value: string) => {
        setKeyword(value);
        setPagination((prev) => ({ ...prev, page: 1 }));
    };

    const { run: debouncedSearch } = useDebounceFn(doSearch, 300);

    const { data: rolePage, isLoading: loadingRoles } = useRoleList(
        {
            page: pagination.page,
            pageSize: pagination.pageSize,
            keyword: keyword || undefined,
        },
        { enabled: canQuery }
    );

    const { data: menuTree = [] } = useMenuTree(undefined, { enabled: canPerm });

    const { data: roleDetail } = useRoleDetail(currentRole?.id ?? 0, {
        enabled: !!currentRole && permModalVisible && canPerm,
    });

    const saveMutation = useRoleSave();
    const deleteMutation = useRoleDelete();
    const updatePermMutation = useRoleUpdate();

    const treeData = useMemo(() => menuTreeToTreeData(menuTree), [menuTree]);
    const allKeys = useMemo(() => getAllTreeKeys(menuTree), [menuTree]);
    const parentKeys = useMemo(() => getParentKeys(menuTree), [menuTree]);

    const checkedKeys = useMemo(() => {
        if (userCheckedKeys !== null) return userCheckedKeys;
        if (!roleDetail || !permModalVisible || !roleDetail.menu_ids) return [];
        return roleDetail.menu_ids.filter((id: number) => !parentKeys.has(id));
    }, [userCheckedKeys, roleDetail, permModalVisible, parentKeys]);

    const expandedKeys = useMemo(() => {
        if (userExpandedKeys !== null) return userExpandedKeys;
        if (!roleDetail || !permModalVisible) return [];
        return allKeys;
    }, [userExpandedKeys, roleDetail, permModalVisible, allKeys]);

    const openCreateModal = () => {
        setEditing(null);
        setModalVisible(true);
    };

    const openEditModal = (record: Role.Item) => {
        setEditing(record);
        setModalVisible(true);
    };

    const handleModalAfterOpen = (open: boolean) => {
        if (!open) {
            form.resetFields();
            return;
        }
        if (editing) {
            form.setFieldsValue({
                id: editing.id,
                name: editing.name,
                code: editing.code,
                status: editing.status,
            });
        } else {
            form.resetFields();
            form.setFieldsValue({ status: 'enabled' });
        }
    };

    const openPermModal = (record: Role.Item) => {
        setCurrentRole(record);
        setUserCheckedKeys(null);
        setUserExpandedKeys(null);
        setPermModalVisible(true);
    };

    const onDelete = (record: Role.Item) => {
        modal.confirm({
            title: `确认删除角色「${record.name}」吗？`,
            content: '删除后拥有该角色的用户将失去对应权限。此操作不可撤销。',
            okText: '确定删除',
            cancelText: '取消',
            okButtonProps: { danger: true },
            onOk: () => deleteMutation.mutate(record.id),
        });
    };

    const onFinish = (values: RoleFormValues) => {
        saveMutation.mutate(values, {
            onSuccess: () => {
                setModalVisible(false);
                setEditing(null);
            },
        });
    };

    const handlePageChange = (page: number, pageSize: number) => {
        setPagination({
            page,
            pageSize,
        });
    };

    const onTreeCheck: TreeProps['onCheck'] = (checked) => {
        setUserCheckedKeys(checked as number[]);
    };

    const handleSavePerm = () => {
        if (!currentRole) return;

        const menuIds = new Set<number>(checkedKeys);

        const addParents = (id: number) => {
            const findParent = (nodes: Menu.TreeItem[], targetId: number): number | null => {
                for (const node of nodes) {
                    if (node.children?.some((c) => c.id === targetId)) {
                        return node.id;
                    }
                    if (node.children) {
                        const found = findParent(node.children, targetId);
                        if (found) return found;
                    }
                }
                return null;
            };

            const parentId = findParent(menuTree, id);
            if (parentId && !menuIds.has(parentId)) {
                menuIds.add(parentId);
                addParents(parentId);
            }
        };

        checkedKeys.forEach(addParents);

        updatePermMutation.mutate(
            {
                id: currentRole.id,
                data: { menu_ids: Array.from(menuIds) },
            },
            {
                onSuccess: () => {
                    setPermModalVisible(false);
                    setCurrentRole(null);
                    setUserCheckedKeys(null);
                    setUserExpandedKeys(null);
                },
            }
        );
    };

    const handleSelectAll = () => {
        const leafKeys = allKeys.filter((k) => !parentKeys.has(k));
        if (checkedKeys.length === leafKeys.length) {
            setUserCheckedKeys([]);
        } else {
            setUserCheckedKeys(leafKeys);
        }
    };

    const handleExpandAll = () => {
        if (expandedKeys.length === allKeys.length) {
            setUserExpandedKeys([]);
        } else {
            setUserExpandedKeys(allKeys);
        }
    };

    if (!canQuery) {
        return (
            <PageContainer>
                <Result
                    status="403"
                    title="无权限"
                    subTitle="您没有查询角色列表的权限，请联系管理员"
                />
            </PageContainer>
        );
    }

    const columns: ColumnsType<Role.Item> = [
        { title: '角色名称', dataIndex: 'name' },
        { title: '角色编码', dataIndex: 'code' },
        {
            title: '状态',
            dataIndex: 'status',
            render: (v: Role.Status) => <StatusTag status={v} />,
        },
        {
            title: '权限数量',
            dataIndex: 'menu_ids',
            render: (ids: number[]) => <Tag color="blue">{ids?.length || 0} 个</Tag>,
        },
        {
            title: '操作',
            key: 'actions',
            width: 240,
            fixed: 'right' as const,
            render: (_, record) => {
                const isSuperadmin = record.code === 'superadmin';
                return (
                    <Space>
                        {canPerm && (
                            <Button
                                type="link"
                                onClick={() => openPermModal(record)}
                                disabled={isSuperadmin}
                            >
                                权限配置
                            </Button>
                        )}
                        {canEdit && (
                            <Button
                                type="link"
                                onClick={() => openEditModal(record)}
                                disabled={isSuperadmin}
                            >
                                编辑
                            </Button>
                        )}
                        {canDelete && (
                            <Button
                                type="link"
                                danger
                                onClick={() => onDelete(record)}
                                disabled={isSuperadmin}
                            >
                                删除
                            </Button>
                        )}
                    </Space>
                );
            },
        },
    ];

    return (
        <PageContainer
            header={
                <div className="flex flex-wrap items-center justify-between gap-2">
                    <h3 className="m-0 text-base font-medium">角色管理</h3>
                    <Space wrap>
                        <Search
                            allowClear
                            placeholder="角色名称 / 编码"
                            onChange={(e) => debouncedSearch(e.target.value)}
                            onSearch={doSearch}
                            className="w-[220px]"
                        />
                        {canAdd && (
                            <Button type="primary" onClick={openCreateModal}>
                                新建角色
                            </Button>
                        )}
                    </Space>
                </div>
            }
            footer={
                <div className="flex justify-end">
                    <Pagination
                        current={pagination.page}
                        pageSize={pagination.pageSize}
                        total={rolePage?.total || 0}
                        showSizeChanger
                        showTotal={(total, range) => `${range[0]}-${range[1]} / 共 ${total} 条`}
                        onChange={handlePageChange}
                    />
                </div>
            }
        >
            <Table<Role.Item>
                rowKey="id"
                columns={columns}
                dataSource={rolePage?.list || []}
                loading={loadingRoles}
                pagination={false}
                size="middle"
                scroll={{ x: 'max-content' }}
                sticky
            />

            <Modal
                open={modalVisible}
                title={editing ? '编辑角色' : '新建角色'}
                okText="确定"
                cancelText="取消"
                onCancel={() => {
                    setModalVisible(false);
                    setEditing(null);
                }}
                onOk={() => form.submit()}
                confirmLoading={saveMutation.isPending}
                afterOpenChange={handleModalAfterOpen}
                destroyOnHidden
                width={480}
            >
                <Form<RoleFormValues> form={form} layout="vertical" onFinish={onFinish}>
                    <Form.Item name="id" hidden>
                        <Input />
                    </Form.Item>
                    <Form.Item
                        label="角色名称"
                        name="name"
                        rules={[{ required: true, message: '请输入角色名称' }]}
                    >
                        <Input placeholder="例如：系统管理员" />
                    </Form.Item>
                    <Form.Item
                        label="角色编码"
                        name="code"
                        rules={[
                            { required: true, message: '请输入角色编码' },
                            {
                                pattern: /^[a-zA-Z][a-zA-Z0-9_]*$/,
                                message: '编码以字母开头，只能包含字母、数字、下划线',
                            },
                        ]}
                    >
                        <Input
                            placeholder="例如：admin"
                            disabled={editing?.code === 'superadmin'}
                        />
                    </Form.Item>
                    <Form.Item label="状态" name="status" rules={[{ required: true }]}>
                        <Select>
                            <Select.Option value="enabled">启用</Select.Option>
                            <Select.Option value="disabled">禁用</Select.Option>
                        </Select>
                    </Form.Item>
                </Form>
            </Modal>

            <Modal
                open={permModalVisible}
                title={`权限配置 - ${currentRole?.name || ''}`}
                okText="保存"
                cancelText="取消"
                onCancel={() => {
                    setPermModalVisible(false);
                    setCurrentRole(null);
                    setUserCheckedKeys(null);
                    setUserExpandedKeys(null);
                }}
                onOk={handleSavePerm}
                confirmLoading={updatePermMutation.isPending}
                destroyOnHidden
                width={600}
            >
                <div className="mb-4 flex items-center justify-between">
                    <Space>
                        <Button size="small" onClick={handleSelectAll}>
                            {checkedKeys.length === allKeys.length - parentKeys.size
                                ? '取消全选'
                                : '全选'}
                        </Button>
                        <Button size="small" onClick={handleExpandAll}>
                            {expandedKeys.length === allKeys.length ? '折叠全部' : '展开全部'}
                        </Button>
                    </Space>
                    <span className="text-xs text-gray-400">
                        已选 {checkedKeys.length} / {allKeys.length - parentKeys.size} 项权限
                    </span>
                </div>
                <div className="max-h-[400px] overflow-auto rounded border border-gray-100 p-3">
                    {treeData.length > 0 ? (
                        <Tree
                            checkable
                            checkStrictly={false}
                            checkedKeys={checkedKeys}
                            expandedKeys={expandedKeys}
                            onExpand={(keys) => setUserExpandedKeys(keys as number[])}
                            onCheck={onTreeCheck}
                            treeData={treeData}
                        />
                    ) : (
                        <div className="p-10 text-center text-gray-400">暂无菜单数据</div>
                    )}
                </div>
            </Modal>
        </PageContainer>
    );
};

export default SystemRolePage;
