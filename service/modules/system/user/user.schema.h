#pragma once

#include <string_view>

#include <model.h>

namespace service::user {

inline bool isPhoneNumber(std::string_view value) {
    if (value.size() != 11 || value[0] != '1' || value[1] < '3' || value[1] > '9')
        return false;
    for (const char ch : value.substr(2)) {
        if (ch < '0' || ch > '9')
            return false;
    }
    return true;
}

RUVIA_MODEL(CreateUserBody,
            RUVIA_REQUIRED_FIELD(username, ruvia::string,
                                 RUVIA_MIN(2, "用户名长度需在 2 - 50 之间"),
                                 RUVIA_MAX(50, "用户名长度需在 2 - 50 之间")),
            RUVIA_REQUIRED_FIELD(password, ruvia::string, RUVIA_MIN(6, "密码长度需在 6 - 100 之间"),
                                 RUVIA_MAX(100, "密码长度需在 6 - 100 之间")),
            RUVIA_OPTIONAL_FIELD(nickname, ruvia::string),
            RUVIA_OPTIONAL_FIELD(phone, ruvia::string,
                                 RUVIA_CUSTOM(isPhoneNumber, "手机号格式不正确")),
            RUVIA_OPTIONAL_FIELD(email, ruvia::string, RUVIA_EMAIL("邮箱格式不正确")),
            RUVIA_OPTIONAL_FIELD_NAME("dept_id", deptId, ruvia::int64),
            RUVIA_OPTIONAL_FIELD(status, ruvia::string),
            RUVIA_REQUIRED_FIELD_NAME("role_ids", roleIds, ruvia::array<ruvia::int64>,
                                      RUVIA_MIN(1, "至少选择一个角色")));

RUVIA_MODEL(UpdateUserBody, RUVIA_OPTIONAL_FIELD(nickname, ruvia::string),
            RUVIA_OPTIONAL_FIELD(phone, ruvia::string,
                                 RUVIA_CUSTOM(isPhoneNumber, "手机号格式不正确")),
            RUVIA_OPTIONAL_FIELD(email, ruvia::string, RUVIA_EMAIL("邮箱格式不正确")),
            RUVIA_OPTIONAL_FIELD_NAME("dept_id", deptId, ruvia::int64),
            RUVIA_OPTIONAL_FIELD(status, ruvia::string),
            RUVIA_OPTIONAL_FIELD(password, ruvia::string),
            RUVIA_OPTIONAL_FIELD_NAME("role_ids", roleIds, ruvia::array<ruvia::int64>));

} // namespace service::user
