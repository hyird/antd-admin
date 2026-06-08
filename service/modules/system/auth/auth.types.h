#pragma once

#include <cstdint>
#include <cyra/http/Model.h>

#include "service/modules/system/menu/menu.types.h"
#include "service/modules/system/role/role.types.h"

namespace service::auth {

CYRA_MODEL(LoginBody, CYRA_FIELD(username, cyra::String), CYRA_FIELD(password, cyra::String));

CYRA_MODEL(RefreshBody, CYRA_FIELD_NAME("refresh_token", refreshToken, cyra::String));

CYRA_MODEL(AuthUserInfoDto, CYRA_FIELD(id, cyra::Int64), CYRA_FIELD(username, cyra::String),
           CYRA_FIELD(nickname, cyra::String, CYRA_OMIT_EMPTY), CYRA_FIELD(status, cyra::String),
           CYRA_FIELD(roles, cyra::Array<role::RoleOptionDto>),
           CYRA_FIELD(menus, cyra::List<menu::MenuDto>));

CYRA_MODEL(LoginResultDto, CYRA_FIELD(token, cyra::String),
           CYRA_FIELD_NAME("refresh_token", refreshToken, cyra::String),
           CYRA_FIELD(user, AuthUserInfoDto));

CYRA_MODEL(LoginResponse, CYRA_FIELD(code, cyra::Int64), CYRA_FIELD(message, cyra::String),
           CYRA_FIELD(data, LoginResultDto));

CYRA_MODEL(CurrentUserResponse, CYRA_FIELD(code, cyra::Int64), CYRA_FIELD(message, cyra::String),
           CYRA_FIELD(data, AuthUserInfoDto));

} // namespace service::auth
