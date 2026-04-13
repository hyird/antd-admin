/**
 * 菜单管理 Service
 */

import { useSaveMutation } from '@/hooks/useMutation';
import type { Menu } from './menu.types';
import { create, update } from './menu.api';
import { menuQueryKeys } from './menu.types';

export {
    getList,
    getTree,
    getDetail,
    create,
    update,
    remove,
    reorder,
    batchCreateButtons,
} from './menu.api';

export function useMenuSave() {
    return useSaveMutation<Menu.CreateDto & { id?: number }, Menu.CreateDto, Menu.UpdateDto>({
        createFn: create,
        updateFn: update,
        toUpdatePayload: ({ id: _id, ...data }) => data,
        createMessage: '保存成功',
        updateMessage: '保存成功',
        invalidateKeys: [menuQueryKeys.all],
    });
}
