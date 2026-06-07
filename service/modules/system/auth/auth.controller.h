#pragma once

#include <utility>

#include <cyra/app/Task.h>
#include <cyra/http/Context.h>
#include <cyra/http/Controller.h>
#include <cyra/http/HttpTypes.h>

#include "service/common/http.h"
#include "service/common/request.h"
#include "service/middleware/auth.h"
#include "service/modules/system/auth/auth.schema.h"
#include "service/modules/system/auth/auth.service.h"

namespace service::modules::system::auth {

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
        auto result = co_await authService().login(c, c.valid<LoginBody>());

        AuthUserInfoDto user(c);
        user.id(static_cast<cyra::Int64>(result.user.id))
            .username(std::move(result.user.username))
            .nickname(std::move(result.user.nickname))
            .status(std::move(result.user.status))
            .roles(std::move(result.user.roles))
            .menus(std::move(result.user.menus));

        LoginResultDto data(c);
        data.token(std::move(result.token))
            .refreshToken(std::move(result.refresh_token))
            .user(std::move(user));

        co_return c.json(service::common::ok<LoginResponse>(c, std::move(data)));
    }

    cyra::Task<cyra::HttpResponse> refresh(cyra::Context& c) {
        auto result = co_await authService().refresh(c, c.valid<RefreshBody>());

        AuthUserInfoDto user(c);
        user.id(static_cast<cyra::Int64>(result.user.id))
            .username(std::move(result.user.username))
            .nickname(std::move(result.user.nickname))
            .status(std::move(result.user.status))
            .roles(std::move(result.user.roles))
            .menus(std::move(result.user.menus));

        LoginResultDto data(c);
        data.token(std::move(result.token))
            .refreshToken(std::move(result.refresh_token))
            .user(std::move(user));

        co_return c.json(service::common::ok<LoginResponse>(c, std::move(data)));
    }

    cyra::Task<cyra::HttpResponse> logout(cyra::Context& c) {
        co_return c.json(service::common::operation(c, "退出成功"));
    }

    cyra::Task<cyra::HttpResponse> me(cyra::Context& c) {
        const auto& jwt = service::middleware::currentUser(c);
        auto info = co_await authService().getCurrentUser(c, jwt.user_id);

        AuthUserInfoDto data(c);
        data.id(static_cast<cyra::Int64>(info.id))
            .username(std::move(info.username))
            .nickname(std::move(info.nickname))
            .status(std::move(info.status))
            .roles(std::move(info.roles))
            .menus(std::move(info.menus));

        co_return c.json(service::common::ok<CurrentUserResponse>(c, std::move(data)));
    }
};

}  // namespace service::modules::system::auth
