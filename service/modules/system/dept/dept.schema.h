#pragma once

#include <cyra/http/Controller.h>

#include "service/modules/system/dept/dept.types.h"

namespace service::dept {

class CreateDeptValidator final : public cyra::Middleware<CreateDeptValidator> {
    CYRA_VALIDATE_JSON(CreateDeptBody,
                       CYRA_RULE(name, CYRA_REQUIRED("部门名称不能为空"),
                                 CYRA_MIN(1, "部门名称不能为空"),
                                 CYRA_MAX(100, "部门名称最多100个字符")),
                       CYRA_RULE(code, CYRA_MAX(50, "部门编码最多50个字符")),
                       CYRA_RULE_NAME("parent_id", parentId, CYRA_MIN(1, "上级部门不正确")),
                       CYRA_RULE_NAME("sort_order", sortOrder, CYRA_MIN(0, "排序不能小于0")),
                       CYRA_RULE_NAME("leader_id", leaderId, CYRA_MIN(1, "负责人不正确")),
                       CYRA_RULE(status, CYRA_ONE_OF("状态不正确", "enabled", "disabled")))
};

class UpdateDeptValidator final : public cyra::Middleware<UpdateDeptValidator> {
    CYRA_VALIDATE_JSON(UpdateDeptBody,
                       CYRA_RULE(name, CYRA_MIN(1, "部门名称不能为空"),
                                 CYRA_MAX(100, "部门名称最多100个字符")),
                       CYRA_RULE(code, CYRA_MAX(50, "部门编码最多50个字符")),
                       CYRA_RULE_NAME("parent_id", parentId, CYRA_MIN(1, "上级部门不正确")),
                       CYRA_RULE_NAME("sort_order", sortOrder, CYRA_MIN(0, "排序不能小于0")),
                       CYRA_RULE_NAME("leader_id", leaderId, CYRA_MIN(1, "负责人不正确")),
                       CYRA_RULE(status, CYRA_ONE_OF("状态不正确", "enabled", "disabled")))
};

} // namespace service::dept
