#pragma once

#include <task.h>
#include <context.h>
#include <controller.h>

#include "service/common/http.h"
#include "service/common/types.h"
#include "service/middleware/auth.h"
#include "service/middleware/permission.h"
#include "service/modules/system/role/role.schema.h"
#include "service/modules/system/role/role.service.h"

namespace service::role {

class RoleController final {
  public:
    RUVIA_CONTROLLER_GROUP("/api/roles", service::middleware::AuthMiddleware)
    RUVIA_ROUTES_BEGIN(RoleController)
    RUVIA_GET("/", list);
    RUVIA_GET("/all", listAll);
    RUVIA_GET("/:id", detail);
    RUVIA_POST("/", create, ruvia::json_body<CreateRoleBody>);
    RUVIA_PUT("/:id", update, ruvia::json_body<UpdateRoleBody>);
    RUVIA_DELETE("/:id", remove);
    RUVIA_ROUTES_END

  private:
    ruvia::task<ruvia::http_response> list(ruvia::context& c) {
        co_await service::middleware::requirePermission(c, "system:role:query");
        const auto pageSize = service::common::parseInt64(c.request().query("page_size"));
        const auto [page, pageSizeValue, skip, keyword, paginated] =
            service::common::normalizePagination(
                service::common::parseInt64(c.request().query("page")), pageSize,
                c.request().query("keyword"));
        co_return c.json(service::common::ok<RolePageResponse>(co_await roleService().list(
            c, page, pageSizeValue, skip, keyword, paginated, c.request().query("status"))));
    }

    ruvia::task<ruvia::http_response> listAll(ruvia::context& c) {
        co_await service::middleware::requireAnyPermission(c,
                                                           {"system:user:add", "system:user:edit"});
        co_return c.json(
            service::common::ok<RoleOptionsResponse>(co_await roleService().listAllEnabled(c)));
    }

    ruvia::task<ruvia::http_response> detail(ruvia::context& c) {
        co_await service::middleware::requirePermission(c, "system:role:query");
        const auto id = service::common::parseInt64(c.request().param("id"));
        if (!id || *id <= 0)
            service::common::throwAppError(service::common::kValidationErrorCode, "id 必须是正整数",
                                           400);
        co_return c.json(
            service::common::ok<RoleDetailResponse>(co_await roleService().getById(c, *id)));
    }

    ruvia::task<ruvia::http_response> create(ruvia::context& c) {
        co_await service::middleware::requirePermission(c, "system:role:add");
        co_await roleService().create(c, c.request().validated<CreateRoleBody>());
        co_return c.json(service::common::operation("创建成功"));
    }

    ruvia::task<ruvia::http_response> update(ruvia::context& c) {
        co_await service::middleware::requirePermission(c, "system:role:edit");
        const auto id = service::common::parseInt64(c.request().param("id"));
        if (!id || *id <= 0)
            service::common::throwAppError(service::common::kValidationErrorCode, "id 必须是正整数",
                                           400);
        co_await roleService().update(c, *id, c.request().validated<UpdateRoleBody>());
        co_return c.json(service::common::operation("更新成功"));
    }

    ruvia::task<ruvia::http_response> remove(ruvia::context& c) {
        co_await service::middleware::requirePermission(c, "system:role:delete");
        const auto id = service::common::parseInt64(c.request().param("id"));
        if (!id || *id <= 0)
            service::common::throwAppError(service::common::kValidationErrorCode, "id 必须是正整数",
                                           400);
        co_await roleService().remove(c, *id);
        co_return c.json(service::common::operation("删除成功"));
    }
};

} // namespace service::role
