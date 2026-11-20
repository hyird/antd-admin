#pragma once

#include <model.h>

namespace service::dept {

RUVIA_MODEL(
    CreateDeptBody,
    RUVIA_REQUIRED_FIELD(name, ruvia::string, RUVIA_MIN(1, "部门名称不能为空"),
                         RUVIA_MAX(100, "部门名称最多100个字符")),
    RUVIA_OPTIONAL_FIELD(code, ruvia::string, RUVIA_MAX(50, "部门编码最多50个字符")),
    RUVIA_OPTIONAL_FIELD_NAME("parent_id", parentId, ruvia::int64, RUVIA_MIN(1, "上级部门不正确")),
    RUVIA_OPTIONAL_FIELD_NAME("sort_order", sortOrder, ruvia::int64, RUVIA_MIN(0, "排序不能小于0")),
    RUVIA_OPTIONAL_FIELD_NAME("leader_id", leaderId, ruvia::int64, RUVIA_MIN(1, "负责人不正确")),
    RUVIA_OPTIONAL_FIELD(status, ruvia::string, RUVIA_ONE_OF("状态不正确", "enabled", "disabled")));

RUVIA_MODEL(
    UpdateDeptBody,
    RUVIA_OPTIONAL_FIELD(name, ruvia::string, RUVIA_MIN(1, "部门名称不能为空"),
                         RUVIA_MAX(100, "部门名称最多100个字符")),
    RUVIA_OPTIONAL_FIELD(code, ruvia::string, RUVIA_MAX(50, "部门编码最多50个字符")),
    RUVIA_OPTIONAL_FIELD_NAME("parent_id", parentId, ruvia::int64, RUVIA_MIN(1, "上级部门不正确")),
    RUVIA_OPTIONAL_FIELD_NAME("sort_order", sortOrder, ruvia::int64, RUVIA_MIN(0, "排序不能小于0")),
    RUVIA_OPTIONAL_FIELD_NAME("leader_id", leaderId, ruvia::int64, RUVIA_MIN(1, "负责人不正确")),
    RUVIA_OPTIONAL_FIELD(status, ruvia::string, RUVIA_ONE_OF("状态不正确", "enabled", "disabled")));

} // namespace service::dept
