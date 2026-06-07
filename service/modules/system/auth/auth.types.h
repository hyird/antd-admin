#pragma once

#include <cstdint>
#include <memory_resource>
#include <string>

#include <cyra/http/Model.h>

#include "service/modules/system/menu/menu.types.h"
#include "service/modules/system/role/role.types.h"

namespace service::modules::system::auth {

CYRA_MODEL(LoginBody,
    CYRA_FIELD(username, cyra::String,
        CYRA_REQUIRED("用户名不能为空"),
        CYRA_MIN(1, "用户名不能为空")),
    CYRA_FIELD(password, cyra::String,
        CYRA_REQUIRED("密码不能为空"),
        CYRA_MIN(1, "密码不能为空"))
);

CYRA_MODEL(RefreshBody,
    CYRA_FIELD_NAME("refresh_token", refreshToken, cyra::String,
        CYRA_REQUIRED("刷新令牌不能为空"),
        CYRA_MIN(1, "刷新令牌不能为空"))
);

CYRA_MODEL(AuthUserInfoDto,
    CYRA_FIELD(id, cyra::Int64),
    CYRA_FIELD(username, cyra::String),
    CYRA_FIELD(nickname, cyra::String, CYRA_OMIT_EMPTY),
    CYRA_FIELD(status, cyra::String),
    CYRA_FIELD(roles, cyra::Array<role::RoleOptionDto>),
    CYRA_FIELD(menus, cyra::List<menu::MenuDto>)
);

CYRA_MODEL(LoginResultDto,
    CYRA_FIELD(token, cyra::String),
    CYRA_FIELD_NAME("refresh_token", refreshToken, cyra::String),
    CYRA_FIELD(user, AuthUserInfoDto)
);

CYRA_MODEL(LoginResponse,
    CYRA_FIELD(code, cyra::Int64),
    CYRA_FIELD(message, cyra::String),
    CYRA_FIELD(data, LoginResultDto)
);

CYRA_MODEL(CurrentUserResponse,
    CYRA_FIELD(code, cyra::Int64),
    CYRA_FIELD(message, cyra::String),
    CYRA_FIELD(data, AuthUserInfoDto)
);

struct UserInfo {
    explicit UserInfo(std::pmr::memory_resource* resource = std::pmr::get_default_resource())
        : username(resource), nickname(resource), status(resource), roles(resource), menus(resource) {}

    std::int64_t id{0};
    std::pmr::string username;
    std::pmr::string nickname;
    std::pmr::string status;
    bool is_superadmin{false};
    cyra::Array<role::RoleOptionDto> roles;
    cyra::List<menu::MenuDto> menus;
};

struct LoginResult {
    explicit LoginResult(std::pmr::memory_resource* resource = std::pmr::get_default_resource())
        : token(resource), refresh_token(resource), user(resource) {}

    std::pmr::string token;
    std::pmr::string refresh_token;
    UserInfo user;
};

}  // namespace service::modules::system::auth
