#pragma once

#include "service/common/http.h"

namespace service::modules::system::user {

namespace UserError {
inline constexpr service::common::AppErrorDef USERNAME_EXISTS{"USERNAME_EXISTS", "用户名已存在", 400};
inline constexpr service::common::AppErrorDef USER_NOT_FOUND{"USER_NOT_FOUND", "用户不存在", 404};
inline constexpr service::common::AppErrorDef PHONE_EXISTS{"PHONE_EXISTS", "手机号已被使用", 400};
inline constexpr service::common::AppErrorDef EMAIL_EXISTS{"EMAIL_EXISTS", "邮箱已被使用", 400};
inline constexpr service::common::AppErrorDef PHONE_INVALID{"PHONE_INVALID", "手机号格式不正确", 400};
inline constexpr service::common::AppErrorDef EMAIL_INVALID{"EMAIL_INVALID", "邮箱格式不正确", 400};
inline constexpr service::common::AppErrorDef ROLE_REQUIRED{"ROLE_REQUIRED", "用户必须分配至少一个角色", 400};
inline constexpr service::common::AppErrorDef ADMIN_ROLE_PROTECTED{"ADMIN_ROLE_PROTECTED", "内置管理员账户的角色不可修改", 400};
inline constexpr service::common::AppErrorDef ADMIN_DELETE_PROTECTED{"ADMIN_DELETE_PROTECTED", "内置管理员账户不可删除", 400};
}  // namespace UserError

}  // namespace service::modules::system::user
