#pragma once

#include <model.h>

namespace service::role {

RUVIA_MODEL(CreateRoleBody,
            RUVIA_REQUIRED_FIELD(code, ruvia::string, RUVIA_MIN(1, "角色编码不能为空"),
                                 RUVIA_MAX(50, "角色编码最多50个字符")),
            RUVIA_REQUIRED_FIELD(name, ruvia::string, RUVIA_MIN(1, "角色名称不能为空"),
                                 RUVIA_MAX(100, "角色名称最多100个字符")),
            RUVIA_OPTIONAL_FIELD(status, ruvia::string),
            RUVIA_OPTIONAL_FIELD_NAME("menu_ids", menuIds, ruvia::array<ruvia::int64>));

RUVIA_MODEL(UpdateRoleBody,
            RUVIA_OPTIONAL_FIELD(code, ruvia::string, RUVIA_MAX(50, "角色编码最多50个字符")),
            RUVIA_OPTIONAL_FIELD(name, ruvia::string, RUVIA_MAX(100, "角色名称最多100个字符")),
            RUVIA_OPTIONAL_FIELD(status, ruvia::string),
            RUVIA_OPTIONAL_FIELD_NAME("menu_ids", menuIds, ruvia::array<ruvia::int64>));

} // namespace service::role
