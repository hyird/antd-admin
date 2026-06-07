#pragma once

#include "service/common/http.h"

namespace service::modules::system::auth {

namespace AuthError {
inline constexpr service::common::AppErrorDef USER_NOT_FOUND{11001, "用户不存在", 400};
inline constexpr service::common::AppErrorDef USER_DISABLED{11002, "用户已被禁用", 403};
inline constexpr service::common::AppErrorDef PASSWORD_INCORRECT{11003, "用户名或密码错误", 400};
inline constexpr service::common::AppErrorDef UNAUTHORIZED{
    service::common::kAuthUnauthorizedErrorCode, "未登录", 401};
inline constexpr service::common::AppErrorDef TOKEN_EXPIRED{
    service::common::kAuthTokenExpiredErrorCode, "Token已过期", 401};
inline constexpr service::common::AppErrorDef TOKEN_INVALID{
    service::common::kAuthTokenInvalidErrorCode, "Token无效", 401};
inline constexpr service::common::AppErrorDef PERMISSION_DENIED{
    service::common::kAuthPermissionDeniedErrorCode, "无权限", 403};
inline constexpr service::common::AppErrorDef TOO_MANY_ATTEMPTS{
    11008, "登录失败次数过多，账户已被锁定", 429};
}  // namespace AuthError

}  // namespace service::modules::system::auth
