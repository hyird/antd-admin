#pragma once

#include <utility>

#include <cyra/app/Task.h>
#include <cyra/http/Context.h>
#include <cyra/http/Controller.h>
#include <cyra/http/HttpTypes.h>

#include "service/common/http.h"
#include "service/middleware/auth.h"
#include "service/modules/system/auth/auth.schema.h"
#include "service/modules/system/auth/auth.service.h"

namespace service::auth {

class AuthController final : public cyra::Controller<AuthController> {
  public:
    CYRA_CONTROLLER_GROUP("/api/auth")
    CYRA_ROUTES_BEGIN
    CYRA_POST("/login", login, LoginValidator);
    CYRA_POST("/refresh", refresh, RefreshValidator);
    CYRA_POST("/logout", logout);
    CYRA_GET("/me", me, service::middleware::AuthMiddleware);
    CYRA_ROUTES_END

  private:
    cyra::Task<cyra::HttpResponse> login(cyra::Context& c) {
        auto data = co_await authService().login(c, c.valid<LoginBody>());
        co_return c.json(service::common::ok<LoginResponse>(c, std::move(data)));
    }

    cyra::Task<cyra::HttpResponse> refresh(cyra::Context& c) {
        auto data = co_await authService().refresh(c, c.valid<RefreshBody>());
        co_return c.json(service::common::ok<LoginResponse>(c, std::move(data)));
    }

    cyra::Task<cyra::HttpResponse> logout(cyra::Context& c) {
        co_return c.json(service::common::operation(c, "退出成功"));
    }

    cyra::Task<cyra::HttpResponse> me(cyra::Context& c) {
        const auto& jwt = service::middleware::currentUser(c);
        auto data = co_await authService().getCurrentUser(c, jwt.user_id);
        co_return c.json(service::common::ok<CurrentUserResponse>(c, std::move(data)));
    }
};

} // namespace service::auth
