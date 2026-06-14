#pragma once

#include <ruvia/http/Controller.h>

#include "service/modules/system/user/user.types.h"

namespace service::user {

class CreateUserValidator final : public ruvia::Middleware<CreateUserValidator> {
    RUVIA_VALIDATE_JSON(CreateUserBody,
                       RUVIA_RULE(username, RUVIA_REQUIRED("用户名不能为空"),
                                 RUVIA_MIN(2, "用户名长度需在 2 - 50 之间"),
                                 RUVIA_MAX(50, "用户名长度需在 2 - 50 之间")),
                       RUVIA_RULE(password, RUVIA_REQUIRED("密码不能为空"),
                                 RUVIA_MIN(6, "密码长度需在 6 - 100 之间"),
                                 RUVIA_MAX(100, "密码长度需在 6 - 100 之间")),
                       RUVIA_RULE(phone, RUVIA_MATCH("手机号格式不正确", isPhoneNumber)),
                       RUVIA_RULE(email, RUVIA_EMAIL("邮箱格式不正确")),
                       RUVIA_RULE_NAME("role_ids", roleIds, RUVIA_REQUIRED("至少选择一个角色"),
                                      RUVIA_MIN(1, "至少选择一个角色")))
};

class UpdateUserValidator final : public ruvia::Middleware<UpdateUserValidator> {
    RUVIA_VALIDATE_JSON(UpdateUserBody,
                       RUVIA_RULE(phone, RUVIA_MATCH("手机号格式不正确", isPhoneNumber)),
                       RUVIA_RULE(email, RUVIA_EMAIL("邮箱格式不正确")))
};

} // namespace service::user
