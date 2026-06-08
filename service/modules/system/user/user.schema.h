#pragma once

#include <cyra/http/Controller.h>

#include "service/modules/system/user/user.types.h"

namespace service::user {

class CreateUserValidator final : public cyra::Middleware<CreateUserValidator> {
    CYRA_VALIDATE_JSON(CreateUserBody,
                       CYRA_RULE(username, CYRA_REQUIRED("用户名不能为空"),
                                 CYRA_MIN(2, "用户名长度需在 2 - 50 之间"),
                                 CYRA_MAX(50, "用户名长度需在 2 - 50 之间")),
                       CYRA_RULE(password, CYRA_REQUIRED("密码不能为空"),
                                 CYRA_MIN(6, "密码长度需在 6 - 100 之间"),
                                 CYRA_MAX(100, "密码长度需在 6 - 100 之间")),
                       CYRA_RULE(phone, CYRA_MATCH("手机号格式不正确", isPhoneNumber)),
                       CYRA_RULE(email, CYRA_EMAIL("邮箱格式不正确")),
                       CYRA_RULE_NAME("role_ids", roleIds, CYRA_REQUIRED("至少选择一个角色"),
                                      CYRA_MIN(1, "至少选择一个角色")))
};

class UpdateUserValidator final : public cyra::Middleware<UpdateUserValidator> {
    CYRA_VALIDATE_JSON(UpdateUserBody,
                       CYRA_RULE(phone, CYRA_MATCH("手机号格式不正确", isPhoneNumber)),
                       CYRA_RULE(email, CYRA_EMAIL("邮箱格式不正确")))
};

} // namespace service::user
