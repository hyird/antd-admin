#pragma once

#include <ruvia/app/Task.h>
#include <ruvia/http/Context.h>
#include <ruvia/http/Controller.h>
#include <ruvia/http/HttpTypes.h>

#include "service/common/http.h"
#include "service/common/types.h"
#include "service/middleware/auth.h"
#include "service/middleware/permission.h"
#include "service/modules/system/user/user.schema.h"
#include "service/modules/system/user/user.service.h"

namespace service::user {

class UserController final : public ruvia::Controller<UserController> {
  public:
    RUVIA_CONTROLLER_GROUP("/api/users", service::middleware::AuthMiddleware)
    RUVIA_ROUTES_BEGIN
    RUVIA_GET("/", list);
    RUVIA_GET("/options", options);
    RUVIA_GET("/:id", detail);
    RUVIA_POST("/", create, CreateUserValidator);
    RUVIA_PUT("/:id", update, UpdateUserValidator);
    RUVIA_DELETE("/:id", remove);
    RUVIA_ROUTES_END

  private:
    ruvia::Task<ruvia::HttpResponse> list(ruvia::Context& c) {
        co_await service::middleware::requirePermission(c, "system:user:query");
        auto pageSize = c.query("pageSize").toInt64();
        if (!pageSize)
            pageSize = c.query("page_size").toInt64();
        const auto [page, pageSizeValue, skip, keyword, paginated] =
            service::common::normalizePagination(c.query("page").toInt64(), pageSize,
                                                 c.query("keyword").toStringView());
        co_return c.json(service::common::ok<UserPageResponse>(
            c, co_await userService().list(c, page, pageSizeValue, skip, keyword, paginated,
                                           c.query("status").toStringView(),
                                           c.query("dept_id").toInt64())));
    }

    ruvia::Task<ruvia::HttpResponse> options(ruvia::Context& c) {
        co_await service::middleware::requireAnyPermission(
            c, {"system:user:query", "system:user:add", "system:user:edit", "system:dept:query",
                "system:dept:add", "system:dept:edit"});
        co_return c.json(service::common::ok<UserOptionsResponse>(
            c, co_await userService().listOptions(c, c.query("keyword").toStringView())));
    }

    ruvia::Task<ruvia::HttpResponse> detail(ruvia::Context& c) {
        co_await service::middleware::requirePermission(c, "system:user:query");
        const auto id = c.param("id").toInt64();
        if (!id || *id <= 0)
            service::common::throwAppError(service::common::kValidationErrorCode, "id 必须是正整数",
                                           400);
        co_return c.json(
            service::common::ok<UserDetailResponse>(c, co_await userService().getById(c, *id)));
    }

    ruvia::Task<ruvia::HttpResponse> create(ruvia::Context& c) {
        co_await service::middleware::requirePermission(c, "system:user:add");
        co_await userService().create(c, c.valid<CreateUserBody>());
        co_return c.json(service::common::operation(c, "创建成功"));
    }

    ruvia::Task<ruvia::HttpResponse> update(ruvia::Context& c) {
        co_await service::middleware::requirePermission(c, "system:user:edit");
        const auto id = c.param("id").toInt64();
        if (!id || *id <= 0)
            service::common::throwAppError(service::common::kValidationErrorCode, "id 必须是正整数",
                                           400);
        co_await userService().update(c, *id, c.valid<UpdateUserBody>());
        co_return c.json(service::common::operation(c, "更新成功"));
    }

    ruvia::Task<ruvia::HttpResponse> remove(ruvia::Context& c) {
        co_await service::middleware::requirePermission(c, "system:user:delete");
        const auto id = c.param("id").toInt64();
        if (!id || *id <= 0)
            service::common::throwAppError(service::common::kValidationErrorCode, "id 必须是正整数",
                                           400);
        co_await userService().remove(c, *id);
        co_return c.json(service::common::operation(c, "删除成功"));
    }
};

} // namespace service::user
