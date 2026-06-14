#pragma once

#include <ruvia/http/Controller.h>

#include "service/modules/system/menu/menu.types.h"

namespace service::menu {

class CreateMenuValidator final : public ruvia::Middleware<CreateMenuValidator> {
    RUVIA_VALIDATE_JSON(CreateMenuBody,
                       RUVIA_RULE(name, RUVIA_REQUIRED("菜单名称不能为空"),
                                 RUVIA_MIN(1, "菜单名称不能为空"),
                                 RUVIA_MAX(100, "菜单名称最多100个字符")),
                       RUVIA_RULE(path, RUVIA_MAX(200, "路径最多200个字符")),
                       RUVIA_RULE(icon, RUVIA_MAX(100, "图标最多100个字符")),
                       RUVIA_RULE_NAME("parent_id", parentId, RUVIA_MIN(1, "父级菜单不正确")),
                       RUVIA_RULE_NAME("sort_order", sortOrder, RUVIA_MIN(0, "排序不能小于0")),
                       RUVIA_RULE(type, RUVIA_ONE_OF("菜单类型不正确", "menu", "page", "button")),
                       RUVIA_RULE(component, RUVIA_MAX(200, "组件路径最多200个字符")),
                       RUVIA_RULE(status, RUVIA_ONE_OF("状态不正确", "enabled", "disabled")),
                       RUVIA_RULE_NAME("permission_code", permissionCode,
                                      RUVIA_MAX(100, "权限编码最多100个字符")))
};

class UpdateMenuValidator final : public ruvia::Middleware<UpdateMenuValidator> {
    RUVIA_VALIDATE_JSON(UpdateMenuBody,
                       RUVIA_RULE(name, RUVIA_MIN(1, "菜单名称不能为空"),
                                 RUVIA_MAX(100, "菜单名称最多100个字符")),
                       RUVIA_RULE(path, RUVIA_MAX(200, "路径最多200个字符")),
                       RUVIA_RULE(icon, RUVIA_MAX(100, "图标最多100个字符")),
                       RUVIA_RULE_NAME("parent_id", parentId, RUVIA_MIN(1, "父级菜单不正确")),
                       RUVIA_RULE_NAME("sort_order", sortOrder, RUVIA_MIN(0, "排序不能小于0")),
                       RUVIA_RULE(type, RUVIA_ONE_OF("菜单类型不正确", "menu", "page", "button")),
                       RUVIA_RULE(component, RUVIA_MAX(200, "组件路径最多200个字符")),
                       RUVIA_RULE(status, RUVIA_ONE_OF("状态不正确", "enabled", "disabled")),
                       RUVIA_RULE_NAME("permission_code", permissionCode,
                                      RUVIA_MAX(100, "权限编码最多100个字符")))
};

class ReorderMenuItemValidator final : public ruvia::Middleware<ReorderMenuItemValidator> {
    RUVIA_VALIDATE_JSON(ReorderMenuItemBody, RUVIA_RULE(id, RUVIA_REQUIRED("菜单 id 不能为空")),
                       RUVIA_RULE_NAME("sort_order", sortOrder, RUVIA_REQUIRED("排序不能为空")))
};

class ReorderMenuValidator final : public ruvia::Middleware<ReorderMenuValidator> {
    RUVIA_VALIDATE_JSON(ReorderMenuBody, RUVIA_RULE(items, RUVIA_REQUIRED("排序项不能为空"),
                                                  RUVIA_MIN(1, "排序项不能为空"),
                                                  RUVIA_EACH(ReorderMenuItemValidator)))
};

class BatchCreateMenuButtonItemValidator final
    : public ruvia::Middleware<BatchCreateMenuButtonItemValidator> {
    RUVIA_VALIDATE_JSON(BatchCreateMenuButtonItemBody,
                       RUVIA_RULE(name, RUVIA_REQUIRED("按钮名称不能为空"),
                                 RUVIA_MIN(1, "按钮名称不能为空")),
                       RUVIA_RULE_NAME("permission_code", permissionCode,
                                      RUVIA_REQUIRED("权限编码不能为空"),
                                      RUVIA_MIN(1, "权限编码不能为空")))
};

class BatchCreateMenuButtonsValidator final
    : public ruvia::Middleware<BatchCreateMenuButtonsValidator> {
    RUVIA_VALIDATE_JSON(BatchCreateMenuButtonsBody,
                       RUVIA_RULE_NAME("parent_id", parentId, RUVIA_REQUIRED("父级菜单不能为空")),
                       RUVIA_RULE(items, RUVIA_REQUIRED("按钮不能为空"), RUVIA_MIN(1, "按钮不能为空"),
                                 RUVIA_EACH(BatchCreateMenuButtonItemValidator)))
};

} // namespace service::menu
