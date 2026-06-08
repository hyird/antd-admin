#pragma once

#include <cyra/http/Controller.h>

#include "service/modules/system/auth/auth.types.h"

namespace service::auth {

class LoginValidator final : public cyra::Middleware<LoginValidator> {
    CYRA_VALIDATE_JSON(LoginBody,
                       CYRA_RULE(username, CYRA_REQUIRED("用户名不能为空"),
                                 CYRA_MIN(1, "用户名不能为空")),
                       CYRA_RULE(password, CYRA_REQUIRED("密码不能为空"),
                                 CYRA_MIN(1, "密码不能为空")))
};

class RefreshValidator final : public cyra::Middleware<RefreshValidator> {
    CYRA_VALIDATE_JSON(RefreshBody, CYRA_RULE_NAME("refresh_token", refreshToken,
                                                   CYRA_REQUIRED("刷新令牌不能为空"),
                                                   CYRA_MIN(1, "刷新令牌不能为空")))
};

} // namespace service::auth
