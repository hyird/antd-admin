#pragma once

#include "service/common/http.h"

namespace service::modules::system::auth {

namespace AuthError {
inline constexpr service::common::AppErrorDef USER_NOT_FOUND{"USER_NOT_FOUND", "用户不存在", 400};
inline constexpr service::common::AppErrorDef USER_DISABLED{"USER_DISABLED", "用户已被禁用", 403};
inline constexpr service::common::AppErrorDef PASSWORD_INCORRECT{"PASSWORD_INCORRECT", "用户名或密码错误", 400};
inline constexpr service::common::AppErrorDef UNAUTHORIZED{"UNAUTHORIZED", "未登录", 401};
inline constexpr service::common::AppErrorDef TOKEN_EXPIRED{"TOKEN_EXPIRED", "Token已过期", 401};
inline constexpr service::common::AppErrorDef TOKEN_INVALID{"TOKEN_INVALID", "Token无效", 401};
inline constexpr service::common::AppErrorDef PERMISSION_DENIED{"PERMISSION_DENIED", "无权限", 403};
inline constexpr service::common::AppErrorDef TOO_MANY_ATTEMPTS{"TOO_MANY_ATTEMPTS", "登录失败次数过多，账户已被锁定", 429};
}  // namespace AuthError

}  // namespace service::modules::system::auth
