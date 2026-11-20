#pragma once

#include <model.h>

namespace service::auth {

RUVIA_MODEL(
    LoginBody,
    RUVIA_REQUIRED_FIELD(username, ruvia::string, RUVIA_MIN(1, "用户名不能为空")),
    RUVIA_REQUIRED_FIELD(password, ruvia::string, RUVIA_MIN(1, "密码不能为空"))
);

RUVIA_MODEL(
    RefreshBody,
    RUVIA_REQUIRED_FIELD_NAME("refresh_token", refreshToken, ruvia::string, RUVIA_MIN(1, "刷新令牌不能为空"))
);

} // namespace service::auth
