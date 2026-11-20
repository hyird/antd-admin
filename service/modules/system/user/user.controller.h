#pragma once

#include <task.h>
#include <context.h>
#include <controller.h>

#include "service/common/http.h"
#include "service/common/types.h"
#include "service/middleware/auth.h"
#include "service/middleware/permission.h"
#include "service/modules/system/user/user.schema.h"
#include "service/modules/system/user/user.service.h"

namespace service::user {

class UserController final {
  public:
    RUVIA_CONTROLLER_GROUP("/api/users", service::middleware::AuthMiddleware)
    RUVIA_ROUTES_BEGIN(UserController)
    RUVIA_GET("/", list);
    RUVIA_GET("/options", options);
    RUVIA_GET("/:id", detail);
    RUVIA_POST("/", create, ruvia::json_body<CreateUserBody>);
    RUVIA_PUT("/:id", update, ruvia::json_body<UpdateUserBody>);
    RUVIA_DELETE("/:id", remove);
    RUVIA_ROUTES_END

  private:
    ruvia::task<ruvia::http_response> list(ruvia::context& c) {
        co_await service::middleware::requirePermission(c, "system:user:query");
        const auto pageSize = service::common::parseInt64(c.request().query("page_size"));
        const auto [page, pageSizeValue, skip, keyword, paginated] =
            service::common::normalizePagination(
                service::common::parseInt64(c.request().query("page")), pageSize,
                c.request().query("keyword"));
        co_return c.json(service::common::ok<UserPageResponse>(co_await userService().list(
            c, page, pageSizeValue, skip, keyword, paginated, c.request().query("status"),
            service::common::parseInt64(c.request().query("dept_id")))));
    }

    ruvia::task<ruvia::http_response> options(ruvia::context& c) {
        co_await service::middleware::requireAnyPermission(
            c, {"system:user:query", "system:user:add", "system:user:edit", "system:dept:query",
                "system:dept:add", "system:dept:edit"});
        co_return c.json(service::common::ok<UserOptionsResponse>(
            co_await userService().listOptions(c, c.request().query("keyword"))));
    }

    ruvia::task<ruvia::http_response> detail(ruvia::context& c) {
        co_await service::middleware::requirePermission(c, "system:user:query");
        const auto id = service::common::parseInt64(c.request().param("id"));
        if (!id || *id <= 0)
            service::common::throwAppError(service::common::kValidationErrorCode, "id 必须是正整数",
                                           400);
        co_return c.json(
            service::common::ok<UserDetailResponse>(co_await userService().getById(c, *id)));
    }

    ruvia::task<ruvia::http_response> create(ruvia::context& c) {
        co_await service::middleware::requirePermission(c, "system:user:add");
        co_await userService().create(c, c.request().validated<CreateUserBody>());
        co_return c.json(service::common::operation("创建成功"));
    }

    ruvia::task<ruvia::http_response> update(ruvia::context& c) {
        co_await service::middleware::requirePermission(c, "system:user:edit");
        const auto id = service::common::parseInt64(c.request().param("id"));
        if (!id || *id <= 0)
            service::common::throwAppError(service::common::kValidationErrorCode, "id 必须是正整数",
                                           400);
        co_await userService().update(c, *id, c.request().validated<UpdateUserBody>());
        co_return c.json(service::common::operation("更新成功"));
    }

    ruvia::task<ruvia::http_response> remove(ruvia::context& c) {
        co_await service::middleware::requirePermission(c, "system:user:delete");
        const auto id = service::common::parseInt64(c.request().param("id"));
        if (!id || *id <= 0)
            service::common::throwAppError(service::common::kValidationErrorCode, "id 必须是正整数",
                                           400);
        co_await userService().remove(c, *id);
        co_return c.json(service::common::operation("删除成功"));
    }
};

} // namespace service::user
