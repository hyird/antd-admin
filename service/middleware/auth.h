#pragma once

#include <string>
#include <string_view>

#include <task.h>
#include <context.h>
#include <middleware.h>

#include "service/common/http.h"
#include "service/utils/jwt.h"

namespace service::middleware {

// 路由处理器内调用，校验 Authorization 头并解析 JWT；失败抛出 AppError。
inline service::core::JwtPayload requireAuth(ruvia::context& c) {
    const auto authHeader = c.request().header("Authorization");
    if (!authHeader || authHeader->empty()) {
        service::common::throwAppError(service::common::kAuthUnauthorizedErrorCode, "未登录", 401);
    }
    constexpr std::string_view bearer = "Bearer ";
    if (authHeader->size() <= bearer.size() || authHeader->substr(0, bearer.size()) != bearer) {
        service::common::throwAppError(service::common::kAuthUnauthorizedErrorCode, "未登录", 401);
    }
    const std::string token(authHeader->substr(bearer.size()));
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

inline const service::core::JwtPayload& currentUser(ruvia::context& c) {
    return c.request_state<service::core::JwtPayload>();
}

class AuthMiddleware final {
  public:
    ruvia::task<void> invoke(ruvia::context& c, ruvia::next& next) {
        const auto principal = requireAuth(c);
        const auto binding = c.bind_request_state(principal);
        co_await next();
    }
};

} // namespace service::middleware
