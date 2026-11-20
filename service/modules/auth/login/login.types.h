#pragma once

#include <cstdint>
#include <model.h>

#include "service/modules/system/menu/menu.types.h"
#include "service/modules/system/role/role.types.h"

namespace service::auth {

RUVIA_MODEL(
    AuthUserInfoDto,
    RUVIA_OPTIONAL_FIELD(id, ruvia::int64),
    RUVIA_OPTIONAL_FIELD(username, ruvia::string),
    RUVIA_OPTIONAL_FIELD(nickname, ruvia::string, RUVIA_OMIT_EMPTY),
    RUVIA_OPTIONAL_FIELD(status, ruvia::string),
    RUVIA_OPTIONAL_FIELD(roles, ruvia::array<role::RoleOptionDto>),
    RUVIA_OPTIONAL_FIELD(menus, ruvia::boxed_array<menu::MenuDto>)
);

RUVIA_MODEL(
    LoginResultDto,
    RUVIA_OPTIONAL_FIELD(token, ruvia::string),
    RUVIA_OPTIONAL_FIELD_NAME("refresh_token", refreshToken, ruvia::string),
    RUVIA_OPTIONAL_FIELD(user, AuthUserInfoDto)
);

RUVIA_MODEL(
    LoginResponse,
    RUVIA_OPTIONAL_FIELD(code, ruvia::int64),
    RUVIA_OPTIONAL_FIELD(message, ruvia::string),
    RUVIA_OPTIONAL_FIELD(data, LoginResultDto)
);

RUVIA_MODEL(
    CurrentUserResponse,
    RUVIA_OPTIONAL_FIELD(code, ruvia::int64),
    RUVIA_OPTIONAL_FIELD(message, ruvia::string),
    RUVIA_OPTIONAL_FIELD(data, AuthUserInfoDto)
);

} // namespace service::auth
