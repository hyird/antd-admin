#pragma once

#include "service/common/http.h"

namespace service::modules::system::user {

namespace UserError {
inline constexpr service::common::AppErrorDef USERNAME_EXISTS{12001, "用户名已存在", 400};
inline constexpr service::common::AppErrorDef USER_NOT_FOUND{12002, "用户不存在", 404};
inline constexpr service::common::AppErrorDef PHONE_EXISTS{12003, "手机号已被使用", 400};
inline constexpr service::common::AppErrorDef EMAIL_EXISTS{12004, "邮箱已被使用", 400};
inline constexpr service::common::AppErrorDef PHONE_INVALID{12005, "手机号格式不正确", 400};
inline constexpr service::common::AppErrorDef EMAIL_INVALID{12006, "邮箱格式不正确", 400};
inline constexpr service::common::AppErrorDef ROLE_REQUIRED{12007, "用户必须分配至少一个角色", 400};
inline constexpr service::common::AppErrorDef ADMIN_ROLE_PROTECTED{
    12008, "内置管理员账户的角色不可修改", 400};
inline constexpr service::common::AppErrorDef ADMIN_DELETE_PROTECTED{
    12009, "内置管理员账户不可删除", 400};
}  // namespace UserError

}  // namespace service::modules::system::user
