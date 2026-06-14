#pragma once

#include <utility>

#include <ruvia/app/Task.h>
#include <ruvia/http/Context.h>
#include <ruvia/http/Controller.h>
#include <ruvia/http/HttpTypes.h>

#include "service/common/http.h"
#include "service/middleware/auth.h"
#include "service/modules/system/auth/auth.schema.h"
#include "service/modules/system/auth/auth.service.h"

namespace service::auth {

class AuthController final : public ruvia::Controller<AuthController> {
  public:
    RUVIA_CONTROLLER_GROUP("/api/auth")
    RUVIA_ROUTES_BEGIN
    RUVIA_POST("/login", login, LoginValidator);
    RUVIA_POST("/refresh", refresh, RefreshValidator);
    RUVIA_POST("/logout", logout);
    RUVIA_GET("/me", me, service::middleware::AuthMiddleware);
    RUVIA_ROUTES_END

  private:
    ruvia::Task<ruvia::HttpResponse> login(ruvia::Context& c) {
        auto data = co_await authService().login(c, c.valid<LoginBody>());
        co_return c.json(service::common::ok<LoginResponse>(c, std::move(data)));
    }

    ruvia::Task<ruvia::HttpResponse> refresh(ruvia::Context& c) {
        auto data = co_await authService().refresh(c, c.valid<RefreshBody>());
        co_return c.json(service::common::ok<LoginResponse>(c, std::move(data)));
    }

    ruvia::Task<ruvia::HttpResponse> logout(ruvia::Context& c) {
        co_return c.json(service::common::operation(c, "退出成功"));
    }

    ruvia::Task<ruvia::HttpResponse> me(ruvia::Context& c) {
        const auto& jwt = service::middleware::currentUser(c);
        auto data = co_await authService().getCurrentUser(c, jwt.user_id);
        co_return c.json(service::common::ok<CurrentUserResponse>(c, std::move(data)));
    }
};

} // namespace service::auth
