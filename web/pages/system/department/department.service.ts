/**
 * 部门管理 Service
 */

import { useSaveMutation } from '@/hooks/useMutation';
import type { Department } from './department.types';
import { create, update } from './department.api';
import { departmentQueryKeys } from './department.types';

export { getList, getTree, getDetail, create, update, remove } from './department.api';

export function useDepartmentSave() {
    return useSaveMutation<
        Department.CreateDto & { id?: number },
        Department.CreateDto,
        Department.UpdateDto
    >({
        createFn: create,
        updateFn: update,
        toUpdatePayload: ({ id: _id, ...data }) => data,
        createMessage: '保存成功',
        updateMessage: '保存成功',
        invalidateKeys: [departmentQueryKeys.all],
    });
}
