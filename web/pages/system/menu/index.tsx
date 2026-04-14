/**
 * 菜单管理
 */

export { menuKeys, menuQueryKeys } from './menu.types';
export {
    useMenuSave,
    useMenuDetail,
    useMenuList,
    useMenuTree,
    useMenuTreeForPermission,
    useMenuTreeSelect,
    getList,
    getTree,
    getDetail,
    create,
    update,
    remove,
    reorder,
    batchCreateButtons,
} from './menu.service';

import { useMutation, useQuery, useQueryClient } from '@tanstack/react-query';
import {
    App,
    Button,
    Form,
    Input,
    InputNumber,
    Modal,
    Result,
    Select,
    Space,
    Switch,
    Table,
    Tag,
    TreeSelect,
    type TreeSelectProps,
} from 'antd';
import type { ColumnsType } from 'antd/es/table';
import type { DragEndEvent } from '@dnd-kit/core';
import { closestCenter, DndContext, PointerSensor, useSensor, useSensors } from '@dnd-kit/core';
import {
    SortableContext,
    useSortable,
    arrayMove,
    verticalListSortingStrategy,
} from '@dnd-kit/sortable';
import { CSS } from '@dnd-kit/utilities';
import { DragOutlined } from '@ant-design/icons';
import { useEffect, useMemo, useState } from 'react';
import DynamicIcon from '@/components/DynamicIcon';
import { FormModal } from '@/components/FormModal';
import { PageContainer } from '@/components/PageContainer';
import { StatusTag } from '@/components/StatusTag';
import { getPageConfig, getRegisteredPages, getRegisteredPermissions } from '@/pages';
import { useDebounceFn } from '@/hooks/useDebounceFn';
import { usePermissions } from '@/hooks/usePermission';
import { loginKeys } from '@/pages/login/login.service';
import { batchCreateButtons, create, getTree, remove, reorder, update } from './menu.api';
import { menuQueryKeys } from './';
import type { Menu } from './menu.types';
import { MenuTypeMap } from './menu.types';
import { filterMenuTree, flattenTree, getPathSegment } from '../../../utils/tree';
import { appIconMap, resolveMenuIconName } from '../../../utils/icon';

const { Search } = Input;

interface MenuFormValues {
    id?: number;
    name: string;
    pathSegment?: string;
    icon?: string;
    component?: string;
    parent_id?: number | null;
    sort_order?: number;
    type: Menu.Type;
    status: Menu.Status;
    permission_code?: string;
}

function typeTag(type: Menu.Type) {
    const config = MenuTypeMap[type];
    return <Tag color={config.color}>{config.text}</Tag>;
}

function getSortItems(record: Menu.TreeItem | null) {
    return [...(record?.children || [])].sort(
        (a, b) => (a.sort_order || 0) - (b.sort_order || 0) || a.id - b.id
    );
}

function SortableChildItem({ item }: { item: Menu.TreeItem }) {
    const { attributes, listeners, setNodeRef, transform, transition, isDragging } = useSortable({
        id: String(item.id),
    });

    return (
        <div
            ref={setNodeRef}
            style={{
                transform: CSS.Translate.toString(transform),
                transition: transition || undefined,
                opacity: isDragging ? 0.6 : 1,
            }}
            className="flex items-center gap-3 rounded-md border border-gray-200 bg-white px-3 py-2"
        >
            <Button
                type="text"
                size="small"
                icon={<DragOutlined />}
                className="cursor-grab text-gray-400"
                {...attributes}
                {...listeners}
            />
            <div className="min-w-0 flex-1">
                <div className="flex items-center gap-2">
                    <span className="truncate font-medium">{item.name}</span>
                    <Tag className="shrink-0">{item.type}</Tag>
                </div>
                <div className="truncate text-xs text-gray-400">
                    排序：{item.sort_order} {item.path ? `· ${item.path}` : ''}
                </div>
            </div>
        </div>
    );
}

function filterOutButtons(nodes: Menu.TreeItem[]): Menu.TreeItem[] {
    return nodes
        .filter((node) => node.type !== 'button')
        .map((node) => {
            const filteredChildren = node.children ? filterOutButtons(node.children) : undefined;
            return {
                ...node,
                children:
                    filteredChildren && filteredChildren.length > 0 ? filteredChildren : undefined,
            };
        });
}

const SystemMenuPage = () => {
    const [keyword, setKeyword] = useState('');
    const [hideButtons, setHideButtons] = useState(true);
    const [modalVisible, setModalVisible] = useState(false);
    const [editing, setEditing] = useState<Menu.TreeItem | null>(null);
    const [form] = Form.useForm<MenuFormValues>();
    const queryClient = useQueryClient();
    const { message, modal } = App.useApp();

    const [permModalVisible, setPermModalVisible] = useState(false);
    const [permTargetPage, setPermTargetPage] = useState<Menu.TreeItem | null>(null);
    const [permSelectedCodes, setPermSelectedCodes] = useState<string[]>([]);
    const [sortModalVisible, setSortModalVisible] = useState(false);
    const [sortParent, setSortParent] = useState<Menu.TreeItem | null>(null);
    const [sortChildren, setSortChildren] = useState<Menu.TreeItem[]>([]);

    const { has } = usePermissions();
    const canQuery = has('system:menu:query');
    const canAdd = has('system:menu:add');
    const canEdit = has('system:menu:edit');
    const canDelete = has('system:menu:delete');

    const doSearch = (value: string) => setKeyword(value);

    const { run: debouncedSearch } = useDebounceFn(doSearch, 300);

    const { data: rawMenuTree = [], isLoading } = useQuery({
        queryKey: menuQueryKeys.tree(),
        queryFn: () => getTree(),
        enabled: canQuery,
    });

    const menuMap = useMemo(() => {
        const flattened = flattenTree(rawMenuTree);
        return Object.fromEntries(flattened.map((item) => [item.id, item]));
    }, [rawMenuTree]);

    const fullMenuMap = useMemo(() => {
        const map: Record<number, Menu.TreeItem> = {};
        const traverse = (nodes: Menu.TreeItem[]) => {
            nodes.forEach((node) => {
                map[node.id] = node;
                if (node.children?.length) traverse(node.children);
            });
        };
        traverse(rawMenuTree);
        return map;
    }, [rawMenuTree]);

    const sortChildCountById = useMemo(() => {
        const map: Record<number, number> = {};
        (Object.values(fullMenuMap) as Menu.TreeItem[]).forEach((item) => {
            map[item.id] = item.children?.length ?? 0;
        });
        return map;
    }, [fullMenuMap]);
    const menuTree = useMemo(() => {
        let tree = filterMenuTree(rawMenuTree, keyword);
        if (hideButtons) {
            tree = filterOutButtons(tree);
        }
        return tree;
    }, [rawMenuTree, keyword, hideButtons]);

    const rootSortItems = useMemo(() => {
        return [...rawMenuTree]
            .filter((item) => item.type !== 'button')
            .sort((a, b) => (a.sort_order || 0) - (b.sort_order || 0) || a.id - b.id);
    }, [rawMenuTree]);

    const parentTreeData = useMemo((): TreeSelectProps['treeData'] => {
        const loop = (nodes: Menu.TreeItem[]): TreeSelectProps['treeData'] =>
            nodes
                .filter((n) => n.type !== 'button')
                .map((n) => ({
                    title: `${n.name} (${n.type})`,
                    value: n.id,
                    children: n.children ? loop(n.children) : undefined,
                }));
        return loop(rawMenuTree);
    }, [rawMenuTree]);

    const watchParentId = Form.useWatch('parent_id', form) as number | null | undefined;
    const watchType = Form.useWatch('type', form) as Menu.Type | undefined;

    const parentType: Menu.Type | undefined = useMemo(() => {
        if (!watchParentId) return undefined;
        return menuMap[watchParentId]?.type;
    }, [watchParentId, menuMap]);

    const availableTypes: Menu.Type[] = useMemo(() => {
        if (!watchParentId) {
            return ['menu', 'page'];
        }
        if (parentType === 'menu') {
            return ['menu', 'page'];
        }
        if (parentType === 'page') {
            return ['button'];
        }
        return ['menu', 'page'];
    }, [watchParentId, parentType]);

    const pageSelectOptions = useMemo(
        () =>
            getRegisteredPages().map((page) => ({
                label: `${page.name} (${page.component})`,
                value: page.component,
                description: page.description,
            })),
        []
    );

    const iconSelectOptions = useMemo(
        () =>
            Object.keys(appIconMap).map((iconName) => ({
                label: (
                    <Space>
                        <DynamicIcon name={iconName} />
                        <span>{iconName}</span>
                    </Space>
                ),
                value: iconName,
            })),
        []
    );

    const availablePermissions = useMemo(() => {
        if (parentType !== 'page' || watchType !== 'button') {
            return getRegisteredPermissions();
        }

        const parentMenu = watchParentId ? menuMap[watchParentId] : undefined;
        if (!parentMenu?.component) {
            return getRegisteredPermissions();
        }

        const pageConfig = getPageConfig(parentMenu.component);
        if (!pageConfig?.permissions) {
            return [];
        }

        return pageConfig.permissions.map((perm) => ({
            ...perm,
            module: pageConfig.module,
            resource: pageConfig.name,
        }));
    }, [parentType, watchType, watchParentId, menuMap]);

    const permissionSelectOptions = useMemo(
        () =>
            availablePermissions.map((perm) => ({
                label: `${perm.name} (${perm.code})`,
                value: perm.code,
                module: perm.module,
                resource: perm.resource,
                description: perm.description,
            })),
        [availablePermissions]
    );

    useEffect(() => {
        if (watchType && !availableTypes.includes(watchType)) {
            form.setFieldValue('type', availableTypes[0]);
        }
    }, [availableTypes, watchType, form]);

    const syncAuthAfterMenuChange = () => {
        queryClient.invalidateQueries({ queryKey: loginKeys.currentUser });
    };

    const sortSensors = useSensors(
        useSensor(PointerSensor, { activationConstraint: { distance: 6 } })
    );

    const openSortModal = (record: Menu.TreeItem) => {
        const fullRecord = fullMenuMap[record.id] || record;
        setSortParent(fullRecord);
        setSortChildren(getSortItems(fullRecord));
        setSortModalVisible(true);
    };

    const openRootSortModal = () => {
        setSortParent(null);
        setSortChildren([...rootSortItems]);
        setSortModalVisible(true);
    };

    const handleSortDragEnd = (event: DragEndEvent) => {
        const { active, over } = event;
        if (!over || active.id === over.id) return;
        const activeIndex = sortChildren.findIndex((item) => String(item.id) === active.id);
        const overIndex = sortChildren.findIndex((item) => String(item.id) === over.id);
        if (activeIndex < 0 || overIndex < 0) return;
        setSortChildren((current) => arrayMove(current, activeIndex, overIndex));
    };

    const sortMutation = useMutation({
        mutationFn: async () => {
            if (sortChildren.length === 0) return;
            await reorder({
                items: sortChildren.map((item, index) => ({
                    id: item.id,
                    parent_id: sortParent ? sortParent.id : null,
                    sort_order: index + 1,
                })),
            });
        },
        onSuccess: async () => {
            message.success('排序成功');
            setSortModalVisible(false);
            setSortParent(null);
            setSortChildren([]);
            queryClient.invalidateQueries({ queryKey: menuQueryKeys.all });
            await syncAuthAfterMenuChange();
        },
    });

    const saveMutation = useMutation({
        mutationFn: async (values: MenuFormValues) => {
            const parent =
                values.parent_id !== undefined && values.parent_id !== null
                    ? menuMap[values.parent_id]
                    : undefined;

            let finalPath: string | null = null;
            if (values.type === 'menu' || values.type === 'page') {
                const seg = (values.pathSegment || '').trim();
                if (seg) {
                    const normalizedSeg = seg.startsWith('/') ? seg : `/${seg}`;
                    const parentPath = (parent?.path || '').trim();
                    if (parentPath) {
                        finalPath = `${parentPath.replace(/\/$/, '')}${normalizedSeg}`;
                    } else {
                        finalPath = normalizedSeg;
                    }
                } else {
                    finalPath = null;
                }
            } else {
                finalPath = null;
            }

            const finalIcon = values.type === 'button' ? '' : (values.icon ?? '').trim();

            if (values.id) {
                const payload: Menu.UpdateDto = {
                    name: values.name,
                    path: finalPath ?? undefined,
                    icon: finalIcon,
                    component: values.component,
                    parent_id: values.parent_id === undefined ? null : values.parent_id,
                    sort_order: values.sort_order,
                    type: values.type,
                    status: values.status,
                    permission_code: values.permission_code,
                };
                await update(values.id, payload);
                return;
            }

            const payload: Menu.CreateDto = {
                name: values.name,
                path: finalPath ?? undefined,
                icon: finalIcon || undefined,
                component: values.component,
                parent_id: values.parent_id === undefined ? null : values.parent_id,
                sort_order: values.sort_order,
                type: values.type,
                status: values.status,
                permission_code: values.permission_code,
            };
            await create(payload);
            return;
        },
        onSuccess: async () => {
            message.success('保存成功');
            setModalVisible(false);
            setEditing(null);
            queryClient.invalidateQueries({ queryKey: menuQueryKeys.all });
            await syncAuthAfterMenuChange();
        },
    });

    const deleteMutation = useMutation({
        mutationFn: (id: number) => remove(id),
        onSuccess: async () => {
            message.success('删除成功');
            queryClient.invalidateQueries({ queryKey: menuQueryKeys.all });
            await syncAuthAfterMenuChange();
        },
    });

    const batchCreatePermMutation = useMutation({
        mutationFn: async ({
            parentPage,
            codes,
        }: {
            parentPage: Menu.TreeItem;
            codes: string[];
        }) => {
            const pageConfig = parentPage.component
                ? getPageConfig(parentPage.component)
                : undefined;
            if (!pageConfig?.permissions) return;

            const existingCodes = new Set(
                (parentPage.children || [])
                    .filter((c) => c.type === 'button' && c.permission_code)
                    .map((c) => c.permission_code!)
            );

            const toCreate = codes.filter((code) => !existingCodes.has(code));
            if (toCreate.length === 0) {
                message.warning('所选权限已存在');
                return 0;
            }

            const permissionMap = new Map(pageConfig.permissions.map((perm) => [perm.code, perm]));
            const payloadItems = toCreate
                .map((code) => permissionMap.get(code))
                .flatMap((perm) => (perm ? [{ name: perm.name, permission_code: perm.code }] : []));

            if (payloadItems.length === 0) {
                return 0;
            }

            const result = await batchCreateButtons({
                parent_id: parentPage.id,
                items: payloadItems,
            });
            return result.created_count;
        },
        onSuccess: async (count) => {
            if (count && count > 0) {
                message.success(`成功添加 ${count} 个权限`);
            }
            setPermModalVisible(false);
            setPermTargetPage(null);
            setPermSelectedCodes([]);
            queryClient.invalidateQueries({ queryKey: menuQueryKeys.all });
            await syncAuthAfterMenuChange();
        },
    });

    const openPermModal = (record: Menu.TreeItem) => {
        const fullRecord = fullMenuMap[record.id] || record;
        setPermTargetPage(fullRecord);
        setPermSelectedCodes([]);
        setPermModalVisible(true);
    };

    const targetPagePermissions = useMemo(() => {
        if (!permTargetPage?.component) return [];
        const pageConfig = getPageConfig(permTargetPage.component);
        if (!pageConfig?.permissions) return [];

        const existingCodes = new Set(
            (permTargetPage.children || [])
                .filter((c) => c.type === 'button' && c.permission_code)
                .map((c) => c.permission_code!)
        );

        return pageConfig.permissions.map((perm) => ({
            ...perm,
            exists: existingCodes.has(perm.code),
        }));
    }, [permTargetPage]);

    const openCreateModal = (parentId?: number | null, defaultType?: Menu.Type) => {
        setEditing(null);
        form.resetFields();
        form.setFieldsValue({
            type: defaultType ?? 'menu',
            status: 'enabled',
            sort_order: 0,
            parent_id: parentId ?? null,
        });
        setModalVisible(true);
    };

    const openEditModal = (record: Menu.TreeItem) => {
        setEditing(record);
        const pathSegment =
            record.type === 'menu' || record.type === 'page' ? getPathSegment(record, menuMap) : '';
        form.setFieldsValue({
            id: record.id,
            name: record.name,
            pathSegment,
            icon: record.icon,
            component: record.component,
            parent_id: record.parent_id ?? null,
            sort_order: record.sort_order,
            type: record.type,
            status: record.status,
            permission_code: record.permission_code,
        });
        setModalVisible(true);
    };

    const onDelete = (record: Menu.TreeItem) => {
        modal.confirm({
            title: `确认删除「${record.name}」吗？`,
            content: '若存在子菜单，请先删除子菜单。',
            okText: '确定删除',
            cancelText: '取消',
            onOk: () => deleteMutation.mutate(record.id),
        });
    };

    const onFinish = (values: MenuFormValues) => {
        saveMutation.mutate(values);
    };

    if (!canQuery) {
        return (
            <PageContainer>
                <Result
                    status="403"
                    title="无权限"
                    subTitle="您没有查询菜单列表的权限，请联系管理员"
                />
            </PageContainer>
        );
    }

    const normalColumns: ColumnsType<Menu.TreeItem> = [
        {
            title: '名称',
            dataIndex: 'name',
        },
        {
            title: '类型',
            dataIndex: 'type',
            render: (t: Menu.Type) => typeTag(t),
        },
        {
            title: '图标',
            dataIndex: 'icon',
            render: (_icon: string | undefined, record) => {
                const icon = resolveMenuIconName(record);
                return icon ? (
                    <Space>
                        <DynamicIcon name={icon} />
                        <span className="text-xs text-gray-400">{icon}</span>
                    </Space>
                ) : (
                    '-'
                );
            },
        },
        {
            title: '完整路由',
            dataIndex: 'path',
            render: (v: string | null, record) => (record.type === 'button' ? '-' : v || '-'),
        },
        {
            title: '组件标识',
            dataIndex: 'component',
            render: (v: string | undefined, record) => (record.type === 'page' ? v || '-' : '-'),
        },
        {
            title: '权限标识',
            dataIndex: 'permission_code',
            render: (v: string | undefined, record) =>
                record.type === 'button'
                    ? v || <span className="text-[#faad14]">未配置</span>
                    : '-',
        },
        {
            title: '排序',
            dataIndex: 'sort_order',
        },
        {
            title: '状态',
            dataIndex: 'status',
            render: (v: Menu.Status) => <StatusTag status={v} />,
        },
        {
            title: '操作',
            key: 'actions',
            width: 320,
            fixed: 'right' as const,
            render: (_, record) => (
                <Space>
                    {canEdit && sortChildCountById[record.id] > 0 && (
                        <Button type="link" onClick={() => openSortModal(record)}>
                            排序
                        </Button>
                    )}
                    {canAdd && record.type === 'menu' && (
                        <Button type="link" onClick={() => openCreateModal(record.id, 'page')}>
                            添加页面
                        </Button>
                    )}
                    {canAdd && record.type === 'page' && record.component && (
                        <Button type="link" onClick={() => openPermModal(record)}>
                            新增权限
                        </Button>
                    )}
                    {canEdit && (
                        <Button type="link" onClick={() => openEditModal(record)}>
                            编辑
                        </Button>
                    )}
                    {canDelete && (
                        <Button type="link" danger onClick={() => onDelete(record)}>
                            删除
                        </Button>
                    )}
                </Space>
            ),
        },
    ];

    return (
        <PageContainer
            header={
                <div className="flex flex-wrap items-center justify-between gap-2">
                    <h3 className="m-0 text-base font-medium">菜单管理</h3>
                    <Space wrap>
                        <Search
                            allowClear
                            placeholder="按名称 / 路由搜索（树状过滤）"
                            onChange={(e) => debouncedSearch(e.target.value)}
                            onSearch={doSearch}
                            className="w-full sm:w-[280px]"
                        />
                        <Switch
                            checked={!hideButtons}
                            onChange={(checked) => setHideButtons(!checked)}
                            checkedChildren="显示权限"
                            unCheckedChildren="隐藏权限"
                        />
                        {canEdit && rootSortItems.length > 1 && (
                            <Button onClick={openRootSortModal}>顶层排序</Button>
                        )}
                        {canAdd && (
                            <Button type="primary" onClick={() => openCreateModal()}>
                                新建菜单
                            </Button>
                        )}
                    </Space>
                </div>
            }
        >
            <Table<Menu.TreeItem>
                rowKey="id"
                columns={normalColumns}
                dataSource={menuTree}
                loading={isLoading}
                pagination={false}
                size="middle"
                scroll={{ x: 'max-content' }}
                expandable={{
                    defaultExpandAllRows: true,
                    rowExpandable: (record) =>
                        Array.isArray(record.children) && record.children.length > 0,
                }}
                sticky
            />

            <FormModal
                open={modalVisible}
                title={editing ? '编辑菜单' : '新建菜单'}
                okText="确定"
                cancelText="取消"
                onCancel={() => {
                    setModalVisible(false);
                    setEditing(null);
                }}
                onOk={() => form.submit()}
                confirmLoading={saveMutation.isPending}
                afterOpenChange={(open) => {
                    if (!open) form.resetFields();
                }}
                destroyOnHidden
                width={640}
            >
                <Form<MenuFormValues> form={form} layout="vertical" onFinish={onFinish}>
                    <Form.Item name="id" hidden>
                        <Input />
                    </Form.Item>

                    <Form.Item
                        label="名称"
                        name="name"
                        rules={[{ required: true, message: '请输入名称' }]}
                    >
                        <Input placeholder="菜单名称 / 页面标题 / 按钮名称" />
                    </Form.Item>

                    <Form.Item label="父级菜单" name="parent_id">
                        <TreeSelect
                            allowClear
                            treeData={parentTreeData}
                            placeholder="不选则为顶级"
                            treeDefaultExpandAll
                        />
                    </Form.Item>

                    <Form.Item
                        label="类型"
                        name="type"
                        rules={[{ required: true, message: '请选择类型' }]}
                    >
                        <Select>
                            {availableTypes.map((t) => (
                                <Select.Option key={t} value={t}>
                                    {t === 'menu' ? '菜单' : t === 'page' ? '页面' : '按钮'}
                                </Select.Option>
                            ))}
                        </Select>
                    </Form.Item>

                    {(watchType === 'menu' || watchType === 'page') && (
                        <Form.Item
                            label="路径片段"
                            name="pathSegment"
                            rules={
                                watchType === 'page'
                                    ? [
                                          {
                                              required: true,
                                              message: '页面必须配置路径片段',
                                          },
                                      ]
                                    : []
                            }
                        >
                            <Input placeholder="例如：system / user / menu（不必带父路径）" />
                        </Form.Item>
                    )}

                    {watchType === 'page' && (
                        <Form.Item
                            label="组件标识"
                            name="component"
                            rules={[
                                {
                                    required: true,
                                    message: '页面必须配置组件标识',
                                },
                            ]}
                        >
                            <Select
                                showSearch
                                allowClear
                                placeholder="选择页面组件"
                                optionFilterProp="label"
                                options={pageSelectOptions}
                                optionRender={(option) => (
                                    <div>
                                        <div>{option.label}</div>
                                        {option.data.description && (
                                            <div className="text-xs text-gray-400">
                                                {option.data.description}
                                            </div>
                                        )}
                                    </div>
                                )}
                            />
                        </Form.Item>
                    )}

                    {watchType === 'button' && (
                        <Form.Item
                            label="权限标识"
                            name="permission_code"
                            rules={[
                                {
                                    required: true,
                                    message: '按钮必须配置权限标识',
                                },
                            ]}
                        >
                            <Select
                                showSearch
                                allowClear
                                placeholder="选择权限标识"
                                filterOption={(input, option) =>
                                    (option?.label ?? '')
                                        .toLowerCase()
                                        .includes(input.toLowerCase()) ||
                                    (option?.value ?? '')
                                        .toLowerCase()
                                        .includes(input.toLowerCase())
                                }
                                options={permissionSelectOptions}
                                optionRender={(option) => (
                                    <div>
                                        <div>
                                            <Tag color="blue" className="!mr-1">
                                                {option.data.module}
                                            </Tag>
                                            <Tag color="green" className="!mr-1">
                                                {option.data.resource}
                                            </Tag>
                                            {option.data.label}
                                        </div>
                                        {option.data.description && (
                                            <div className="mt-0.5 text-xs text-gray-400">
                                                {option.data.description}
                                            </div>
                                        )}
                                    </div>
                                )}
                            />
                        </Form.Item>
                    )}

                    {(watchType === 'menu' || watchType === 'page') && (
                        <Form.Item label="图标" name="icon">
                            <Select
                                allowClear
                                showSearch
                                placeholder="选择图标（可留空）"
                                optionFilterProp="value"
                                options={iconSelectOptions}
                            />
                        </Form.Item>
                    )}

                    <Form.Item label="排序" name="sort_order">
                        <InputNumber className="!w-full" />
                    </Form.Item>
                    <Form.Item label="状态" name="status" rules={[{ required: true }]}>
                        <Select>
                            <Select.Option value="enabled">启用</Select.Option>
                            <Select.Option value="disabled">禁用</Select.Option>
                        </Select>
                    </Form.Item>
                </Form>
            </FormModal>

            <Modal
                open={permModalVisible}
                title={`新增权限 - ${permTargetPage?.name || ''}`}
                okText="确定"
                cancelText="取消"
                onCancel={() => {
                    setPermModalVisible(false);
                    setPermTargetPage(null);
                    setPermSelectedCodes([]);
                }}
                onOk={() => {
                    if (permTargetPage && permSelectedCodes.length > 0) {
                        batchCreatePermMutation.mutate({
                            parentPage: permTargetPage,
                            codes: permSelectedCodes,
                        });
                    }
                }}
                okButtonProps={{ disabled: permSelectedCodes.length === 0 }}
                confirmLoading={batchCreatePermMutation.isPending}
                destroyOnHidden
                width={500}
            >
                {targetPagePermissions.length === 0 ? (
                    <div className="py-5 text-center text-gray-400">该页面未配置权限列表</div>
                ) : (
                    <div>
                        <div className="mb-3 text-gray-500">
                            选择要添加的权限（已存在的权限无法重复添加）：
                        </div>
                        <Space orientation="vertical" className="w-full">
                            {targetPagePermissions.map((perm) => (
                                <div
                                    key={perm.code}
                                    className={`flex items-center rounded-md px-3 py-2 ${perm.exists ? 'cursor-not-allowed bg-gray-100 opacity-60' : 'cursor-pointer bg-gray-50'}`}
                                    onClick={() => {
                                        if (perm.exists) return;
                                        setPermSelectedCodes((prev) =>
                                            prev.includes(perm.code)
                                                ? prev.filter((c) => c !== perm.code)
                                                : [...prev, perm.code]
                                        );
                                    }}
                                >
                                    <input
                                        type="checkbox"
                                        checked={permSelectedCodes.includes(perm.code)}
                                        disabled={perm.exists}
                                        onChange={() => {}}
                                        className="mr-3"
                                    />
                                    <div className="flex-1">
                                        <div className="font-medium">
                                            {perm.name}
                                            {perm.exists && (
                                                <Tag color="default" className="!ml-2">
                                                    已存在
                                                </Tag>
                                            )}
                                        </div>
                                        <div className="text-xs text-gray-400">{perm.code}</div>
                                    </div>
                                </div>
                            ))}
                        </Space>
                    </div>
                )}
            </Modal>

            <Modal
                open={sortModalVisible}
                title={sortParent ? `排序子项 - ${sortParent.name}` : '顶层菜单排序'}
                okText="保存排序"
                cancelText="取消"
                onCancel={() => {
                    setSortModalVisible(false);
                    setSortParent(null);
                    setSortChildren([]);
                }}
                onOk={() => sortMutation.mutate()}
                okButtonProps={{ disabled: sortChildren.length === 0 }}
                confirmLoading={sortMutation.isPending}
                destroyOnHidden
                width={560}
            >
                <div className="mb-3 text-gray-500">
                    {sortParent
                        ? '仅支持拖拽当前菜单的直接子项进行排序。'
                        : '仅支持拖拽顶层菜单进行排序。'}
                </div>
                {sortChildren.length === 0 ? (
                    <div className="py-8 text-center text-gray-400">
                        {sortParent ? '该菜单没有可排序的子项' : '没有可排序的顶层菜单'}
                    </div>
                ) : (
                    <DndContext
                        sensors={sortSensors}
                        collisionDetection={closestCenter}
                        onDragEnd={handleSortDragEnd}
                    >
                        <SortableContext
                            items={sortChildren.map((item) => String(item.id))}
                            strategy={verticalListSortingStrategy}
                        >
                            <Space orientation="vertical" className="w-full" size={8}>
                                {sortChildren.map((item) => (
                                    <SortableChildItem key={item.id} item={item} />
                                ))}
                            </Space>
                        </SortableContext>
                    </DndContext>
                )}
            </Modal>
        </PageContainer>
    );
};

export default SystemMenuPage;
