#pragma once

#include <cyra/http/Controller.h>

#include "service/modules/system/role/role.types.h"

namespace service::role {

class CreateRoleValidator final : public cyra::Middleware<CreateRoleValidator> {
    CYRA_VALIDATE_JSON(CreateRoleBody,
                       CYRA_RULE(code, CYRA_REQUIRED("角色编码不能为空"),
                                 CYRA_MIN(1, "角色编码不能为空"),
                                 CYRA_MAX(50, "角色编码最多50个字符")),
                       CYRA_RULE(name, CYRA_REQUIRED("角色名称不能为空"),
                                 CYRA_MIN(1, "角色名称不能为空"),
                                 CYRA_MAX(100, "角色名称最多100个字符")))
};

class UpdateRoleValidator final : public cyra::Middleware<UpdateRoleValidator> {
    CYRA_VALIDATE_JSON(UpdateRoleBody, CYRA_RULE(code, CYRA_MAX(50, "角色编码最多50个字符")),
                       CYRA_RULE(name, CYRA_MAX(100, "角色名称最多100个字符")))
};

} // namespace service::role
