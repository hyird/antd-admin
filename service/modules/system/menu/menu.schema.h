#pragma once

#include <cyra/http/Controller.h>

#include "service/modules/system/menu/menu.types.h"

namespace service::menu {

class CreateMenuValidator final : public cyra::Middleware<CreateMenuValidator> {
    CYRA_VALIDATE_JSON(CreateMenuBody,
                       CYRA_RULE(name, CYRA_REQUIRED("菜单名称不能为空"),
                                 CYRA_MIN(1, "菜单名称不能为空"),
                                 CYRA_MAX(100, "菜单名称最多100个字符")),
                       CYRA_RULE(path, CYRA_MAX(200, "路径最多200个字符")),
                       CYRA_RULE(icon, CYRA_MAX(100, "图标最多100个字符")),
                       CYRA_RULE_NAME("parent_id", parentId, CYRA_MIN(1, "父级菜单不正确")),
                       CYRA_RULE_NAME("sort_order", sortOrder, CYRA_MIN(0, "排序不能小于0")),
                       CYRA_RULE(type, CYRA_ONE_OF("菜单类型不正确", "menu", "page", "button")),
                       CYRA_RULE(component, CYRA_MAX(200, "组件路径最多200个字符")),
                       CYRA_RULE(status, CYRA_ONE_OF("状态不正确", "enabled", "disabled")),
                       CYRA_RULE_NAME("permission_code", permissionCode,
                                      CYRA_MAX(100, "权限编码最多100个字符")))
};

class UpdateMenuValidator final : public cyra::Middleware<UpdateMenuValidator> {
    CYRA_VALIDATE_JSON(UpdateMenuBody,
                       CYRA_RULE(name, CYRA_MIN(1, "菜单名称不能为空"),
                                 CYRA_MAX(100, "菜单名称最多100个字符")),
                       CYRA_RULE(path, CYRA_MAX(200, "路径最多200个字符")),
                       CYRA_RULE(icon, CYRA_MAX(100, "图标最多100个字符")),
                       CYRA_RULE_NAME("parent_id", parentId, CYRA_MIN(1, "父级菜单不正确")),
                       CYRA_RULE_NAME("sort_order", sortOrder, CYRA_MIN(0, "排序不能小于0")),
                       CYRA_RULE(type, CYRA_ONE_OF("菜单类型不正确", "menu", "page", "button")),
                       CYRA_RULE(component, CYRA_MAX(200, "组件路径最多200个字符")),
                       CYRA_RULE(status, CYRA_ONE_OF("状态不正确", "enabled", "disabled")),
                       CYRA_RULE_NAME("permission_code", permissionCode,
                                      CYRA_MAX(100, "权限编码最多100个字符")))
};

class ReorderMenuItemValidator final : public cyra::Middleware<ReorderMenuItemValidator> {
    CYRA_VALIDATE_JSON(ReorderMenuItemBody, CYRA_RULE(id, CYRA_REQUIRED("菜单 id 不能为空")),
                       CYRA_RULE_NAME("sort_order", sortOrder, CYRA_REQUIRED("排序不能为空")))
};

class ReorderMenuValidator final : public cyra::Middleware<ReorderMenuValidator> {
    CYRA_VALIDATE_JSON(ReorderMenuBody, CYRA_RULE(items, CYRA_REQUIRED("排序项不能为空"),
                                                  CYRA_MIN(1, "排序项不能为空"),
                                                  CYRA_EACH(ReorderMenuItemValidator)))
};

class BatchCreateMenuButtonItemValidator final
    : public cyra::Middleware<BatchCreateMenuButtonItemValidator> {
    CYRA_VALIDATE_JSON(BatchCreateMenuButtonItemBody,
                       CYRA_RULE(name, CYRA_REQUIRED("按钮名称不能为空"),
                                 CYRA_MIN(1, "按钮名称不能为空")),
                       CYRA_RULE_NAME("permission_code", permissionCode,
                                      CYRA_REQUIRED("权限编码不能为空"),
                                      CYRA_MIN(1, "权限编码不能为空")))
};

class BatchCreateMenuButtonsValidator final
    : public cyra::Middleware<BatchCreateMenuButtonsValidator> {
    CYRA_VALIDATE_JSON(BatchCreateMenuButtonsBody,
                       CYRA_RULE_NAME("parent_id", parentId, CYRA_REQUIRED("父级菜单不能为空")),
                       CYRA_RULE(items, CYRA_REQUIRED("按钮不能为空"), CYRA_MIN(1, "按钮不能为空"),
                                 CYRA_EACH(BatchCreateMenuButtonItemValidator)))
};

} // namespace service::menu
