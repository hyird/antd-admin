#pragma once

#include <string>
#include <string_view>

#include <cyra/app/Task.h>
#include <cyra/http/Context.h>
#include <cyra/http/Controller.h>
#include <cyra/http/HttpTypes.h>
#include <cyra/http/Validation.h>

#include "service/common/http.h"
#include "service/utils/jwt.h"

namespace service::middleware {

// 路由处理器内调用，校验 Authorization 头并解析 JWT；失败抛出 AppError。
inline service::core::JwtPayload requireAuth(cyra::Context& c) {
    const auto authHeader = c.header("Authorization");
    if (authHeader.empty()) {
        service::common::throwAppError(service::common::kAuthUnauthorizedErrorCode, "未登录", 401);
    }
    constexpr std::string_view bearer = "Bearer ";
    if (authHeader.size() <= bearer.size() || authHeader.substr(0, bearer.size()) != bearer) {
        service::common::throwAppError(service::common::kAuthUnauthorizedErrorCode, "未登录", 401);
    }
    const std::string token(authHeader.substr(bearer.size()));
    try {
        return service::utils::verifyAccessToken(token);
    } catch (const service::utils::JwtExpiredError&) {
        service::common::throwAppError(service::common::kAuthTokenExpiredErrorCode, "Token已过期",
                                       401);
    } catch (const service::utils::JwtInvalidError&) {
        service::common::throwAppError(service::common::kAuthTokenInvalidErrorCode, "Token无效",
                                       401);
    } catch (...) {
        service::common::throwAppError(service::common::kAuthTokenInvalidErrorCode, "Token无效",
                                       401);
    }
}

inline const service::core::JwtPayload& currentUser(cyra::Context& c) {
    return c.valid<service::core::JwtPayload>(cyra::Form);
}

class AuthMiddleware final : public cyra::Middleware<AuthMiddleware> {
  public:
    cyra::Task<cyra::HttpResponse> handle(cyra::Context& c, const cyra::Next& next) {
        c.setValid(cyra::Form, requireAuth(c));
        co_return co_await next(c);
    }
};

} // namespace service::middleware
