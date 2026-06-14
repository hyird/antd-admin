#pragma once

#include <ruvia/http/Controller.h>

#include "service/modules/system/role/role.types.h"

namespace service::role {

class CreateRoleValidator final : public ruvia::Middleware<CreateRoleValidator> {
    RUVIA_VALIDATE_JSON(CreateRoleBody,
                       RUVIA_RULE(code, RUVIA_REQUIRED("角色编码不能为空"),
                                 RUVIA_MIN(1, "角色编码不能为空"),
                                 RUVIA_MAX(50, "角色编码最多50个字符")),
                       RUVIA_RULE(name, RUVIA_REQUIRED("角色名称不能为空"),
                                 RUVIA_MIN(1, "角色名称不能为空"),
                                 RUVIA_MAX(100, "角色名称最多100个字符")))
};

class UpdateRoleValidator final : public ruvia::Middleware<UpdateRoleValidator> {
    RUVIA_VALIDATE_JSON(UpdateRoleBody, RUVIA_RULE(code, RUVIA_MAX(50, "角色编码最多50个字符")),
                       RUVIA_RULE(name, RUVIA_MAX(100, "角色名称最多100个字符")))
};

} // namespace service::role
