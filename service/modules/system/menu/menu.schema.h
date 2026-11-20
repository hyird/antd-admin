#pragma once

#include <model.h>

namespace service::menu {

RUVIA_MODEL(
    CreateMenuBody,
    RUVIA_REQUIRED_FIELD(name, ruvia::string, RUVIA_MIN(1, "菜单名称不能为空"),
                         RUVIA_MAX(100, "菜单名称最多100个字符")),
    RUVIA_OPTIONAL_FIELD(path, ruvia::string, RUVIA_MAX(200, "路径最多200个字符")),
    RUVIA_OPTIONAL_FIELD(icon, ruvia::string, RUVIA_MAX(100, "图标最多100个字符")),
    RUVIA_OPTIONAL_FIELD(component, ruvia::string, RUVIA_MAX(200, "组件路径最多200个字符")),
    RUVIA_OPTIONAL_FIELD_NAME("parent_id", parentId, ruvia::int64, RUVIA_MIN(1, "父级菜单不正确")),
    RUVIA_OPTIONAL_FIELD_NAME("sort_order", sortOrder, ruvia::int64, RUVIA_MIN(0, "排序不能小于0")),
    RUVIA_OPTIONAL_FIELD(type, ruvia::string,
                         RUVIA_ONE_OF("菜单类型不正确", "menu", "page", "button")),
    RUVIA_OPTIONAL_FIELD(status, ruvia::string, RUVIA_ONE_OF("状态不正确", "enabled", "disabled")),
    RUVIA_OPTIONAL_FIELD_NAME("permission_code", permissionCode, ruvia::string,
                              RUVIA_MAX(100, "权限编码最多100个字符")),
    RUVIA_OPTIONAL_FIELD_NAME("is_default", isDefault, ruvia::bool_value));

RUVIA_MODEL(
    UpdateMenuBody,
    RUVIA_OPTIONAL_FIELD(name, ruvia::string, RUVIA_MIN(1, "菜单名称不能为空"),
                         RUVIA_MAX(100, "菜单名称最多100个字符")),
    RUVIA_OPTIONAL_FIELD(path, ruvia::string, RUVIA_MAX(200, "路径最多200个字符")),
    RUVIA_OPTIONAL_FIELD(icon, ruvia::string, RUVIA_MAX(100, "图标最多100个字符")),
    RUVIA_OPTIONAL_FIELD(component, ruvia::string, RUVIA_MAX(200, "组件路径最多200个字符")),
    RUVIA_OPTIONAL_FIELD_NAME("parent_id", parentId, ruvia::int64, RUVIA_MIN(1, "父级菜单不正确")),
    RUVIA_OPTIONAL_FIELD_NAME("sort_order", sortOrder, ruvia::int64, RUVIA_MIN(0, "排序不能小于0")),
    RUVIA_OPTIONAL_FIELD(type, ruvia::string,
                         RUVIA_ONE_OF("菜单类型不正确", "menu", "page", "button")),
    RUVIA_OPTIONAL_FIELD(status, ruvia::string, RUVIA_ONE_OF("状态不正确", "enabled", "disabled")),
    RUVIA_OPTIONAL_FIELD_NAME("permission_code", permissionCode, ruvia::string,
                              RUVIA_MAX(100, "权限编码最多100个字符")),
    RUVIA_OPTIONAL_FIELD_NAME("is_default", isDefault, ruvia::bool_value));

RUVIA_MODEL(ReorderMenuItemBody, RUVIA_REQUIRED_FIELD(id, ruvia::int64),
            RUVIA_REQUIRED_FIELD_NAME("sort_order", sortOrder, ruvia::int64),
            RUVIA_OPTIONAL_FIELD_NAME("parent_id", parentId, ruvia::int64));

RUVIA_MODEL(ReorderMenuBody, RUVIA_REQUIRED_FIELD(items, ruvia::array<ReorderMenuItemBody>,
                                                  RUVIA_MIN(1, "排序项不能为空")));

RUVIA_MODEL(BatchCreateMenuButtonItemBody,
            RUVIA_REQUIRED_FIELD(name, ruvia::string, RUVIA_MIN(1, "按钮名称不能为空")),
            RUVIA_REQUIRED_FIELD_NAME("permission_code", permissionCode, ruvia::string,
                                      RUVIA_MIN(1, "权限编码不能为空")));

RUVIA_MODEL(BatchCreateMenuButtonsBody,
            RUVIA_REQUIRED_FIELD_NAME("parent_id", parentId, ruvia::int64),
            RUVIA_REQUIRED_FIELD(items, ruvia::array<BatchCreateMenuButtonItemBody>,
                                 RUVIA_MIN(1, "按钮不能为空")));

} // namespace service::menu
