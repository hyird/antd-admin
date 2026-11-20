#pragma once

#include <utility>

#include <task.h>
#include <context.h>
#include <controller.h>

#include "service/common/http.h"
#include "service/middleware/auth.h"
#include "service/modules/auth/login/login.schema.h"
#include "service/modules/auth/login/login.service.h"

namespace service::auth {

class AuthController final {
  public:
    RUVIA_CONTROLLER_GROUP("/api/auth")
    RUVIA_ROUTES_BEGIN(AuthController)
    RUVIA_POST("/login", login, ruvia::json_body<LoginBody>);
    RUVIA_POST("/refresh", refresh, ruvia::json_body<RefreshBody>);
    RUVIA_POST("/logout", logout);
    RUVIA_GET("/me", me, service::middleware::AuthMiddleware);
    RUVIA_ROUTES_END

  private:
    ruvia::task<ruvia::http_response> login(ruvia::context& c) {
        auto data = co_await authService().login(c, c.request().validated<LoginBody>());
        co_return c.json(service::common::ok<LoginResponse>(std::move(data)));
    }

    ruvia::task<ruvia::http_response> refresh(ruvia::context& c) {
        auto data = co_await authService().refresh(c, c.request().validated<RefreshBody>());
        co_return c.json(service::common::ok<LoginResponse>(std::move(data)));
    }

    ruvia::task<ruvia::http_response> logout(ruvia::context& c) {
        co_return c.json(service::common::operation("退出成功"));
    }

    ruvia::task<ruvia::http_response> me(ruvia::context& c) {
        const auto& jwt = service::middleware::currentUser(c);
        auto data = co_await authService().getCurrentUser(c, jwt.user_id);
        co_return c.json(service::common::ok<CurrentUserResponse>(std::move(data)));
    }
};

} // namespace service::auth
