#pragma once

#include <ruvia/http/Controller.h>

#include "service/modules/system/dept/dept.types.h"

namespace service::dept {

class CreateDeptValidator final : public ruvia::Middleware<CreateDeptValidator> {
    RUVIA_VALIDATE_JSON(CreateDeptBody,
                       RUVIA_RULE(name, RUVIA_REQUIRED("部门名称不能为空"),
                                 RUVIA_MIN(1, "部门名称不能为空"),
                                 RUVIA_MAX(100, "部门名称最多100个字符")),
                       RUVIA_RULE(code, RUVIA_MAX(50, "部门编码最多50个字符")),
                       RUVIA_RULE_NAME("parent_id", parentId, RUVIA_MIN(1, "上级部门不正确")),
                       RUVIA_RULE_NAME("sort_order", sortOrder, RUVIA_MIN(0, "排序不能小于0")),
                       RUVIA_RULE_NAME("leader_id", leaderId, RUVIA_MIN(1, "负责人不正确")),
                       RUVIA_RULE(status, RUVIA_ONE_OF("状态不正确", "enabled", "disabled")))
};

class UpdateDeptValidator final : public ruvia::Middleware<UpdateDeptValidator> {
    RUVIA_VALIDATE_JSON(UpdateDeptBody,
                       RUVIA_RULE(name, RUVIA_MIN(1, "部门名称不能为空"),
                                 RUVIA_MAX(100, "部门名称最多100个字符")),
                       RUVIA_RULE(code, RUVIA_MAX(50, "部门编码最多50个字符")),
                       RUVIA_RULE_NAME("parent_id", parentId, RUVIA_MIN(1, "上级部门不正确")),
                       RUVIA_RULE_NAME("sort_order", sortOrder, RUVIA_MIN(0, "排序不能小于0")),
                       RUVIA_RULE_NAME("leader_id", leaderId, RUVIA_MIN(1, "负责人不正确")),
                       RUVIA_RULE(status, RUVIA_ONE_OF("状态不正确", "enabled", "disabled")))
};

} // namespace service::dept
