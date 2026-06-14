#pragma once

#include <ruvia/app/Task.h>
#include <ruvia/http/Context.h>
#include <ruvia/http/Controller.h>
#include <ruvia/http/HttpTypes.h>

#include "service/common/http.h"
#include "service/common/types.h"
#include "service/middleware/auth.h"
#include "service/middleware/permission.h"
#include "service/modules/system/role/role.schema.h"
#include "service/modules/system/role/role.service.h"

namespace service::role {

class RoleController final : public ruvia::Controller<RoleController> {
  public:
    RUVIA_CONTROLLER_GROUP("/api/roles", service::middleware::AuthMiddleware)
    RUVIA_ROUTES_BEGIN
    RUVIA_GET("/", list);
    RUVIA_GET("/all", listAll);
    RUVIA_GET("/:id", detail);
    RUVIA_POST("/", create, CreateRoleValidator);
    RUVIA_PUT("/:id", update, UpdateRoleValidator);
    RUVIA_DELETE("/:id", remove);
    RUVIA_ROUTES_END

  private:
    ruvia::Task<ruvia::HttpResponse> list(ruvia::Context& c) {
        co_await service::middleware::requirePermission(c, "system:role:query");
        auto pageSize = c.query("pageSize").toInt64();
        if (!pageSize)
            pageSize = c.query("page_size").toInt64();
        const auto [page, pageSizeValue, skip, keyword, paginated] =
            service::common::normalizePagination(c.query("page").toInt64(), pageSize,
                                                 c.query("keyword").toStringView());
        co_return c.json(service::common::ok<RolePageResponse>(
            c, co_await roleService().list(c, page, pageSizeValue, skip, keyword, paginated,
                                           c.query("status").toStringView())));
    }

    ruvia::Task<ruvia::HttpResponse> listAll(ruvia::Context& c) {
        co_await service::middleware::requireAnyPermission(c,
                                                           {"system:user:add", "system:user:edit"});
        co_return c.json(
            service::common::ok<RoleOptionsResponse>(c, co_await roleService().listAllEnabled(c)));
    }

    ruvia::Task<ruvia::HttpResponse> detail(ruvia::Context& c) {
        co_await service::middleware::requirePermission(c, "system:role:query");
        const auto id = c.param("id").toInt64();
        if (!id || *id <= 0)
            service::common::throwAppError(service::common::kValidationErrorCode, "id 必须是正整数",
                                           400);
        co_return c.json(
            service::common::ok<RoleDetailResponse>(c, co_await roleService().getById(c, *id)));
    }

    ruvia::Task<ruvia::HttpResponse> create(ruvia::Context& c) {
        co_await service::middleware::requirePermission(c, "system:role:add");
        co_await roleService().create(c, c.valid<CreateRoleBody>());
        co_return c.json(service::common::operation(c, "创建成功"));
    }

    ruvia::Task<ruvia::HttpResponse> update(ruvia::Context& c) {
        co_await service::middleware::requirePermission(c, "system:role:edit");
        const auto id = c.param("id").toInt64();
        if (!id || *id <= 0)
            service::common::throwAppError(service::common::kValidationErrorCode, "id 必须是正整数",
                                           400);
        co_await roleService().update(c, *id, c.valid<UpdateRoleBody>());
        co_return c.json(service::common::operation(c, "更新成功"));
    }

    ruvia::Task<ruvia::HttpResponse> remove(ruvia::Context& c) {
        co_await service::middleware::requirePermission(c, "system:role:delete");
        const auto id = c.param("id").toInt64();
        if (!id || *id <= 0)
            service::common::throwAppError(service::common::kValidationErrorCode, "id 必须是正整数",
                                           400);
        co_await roleService().remove(c, *id);
        co_return c.json(service::common::operation(c, "删除成功"));
    }
};

} // namespace service::role
