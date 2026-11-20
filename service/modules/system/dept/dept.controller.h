#pragma once

#include <task.h>
#include <context.h>
#include <controller.h>

#include "service/common/http.h"
#include "service/common/types.h"
#include "service/middleware/auth.h"
#include "service/middleware/permission.h"
#include "service/modules/system/dept/dept.schema.h"
#include "service/modules/system/dept/dept.service.h"

namespace service::dept {

class DeptController final {
  public:
    RUVIA_CONTROLLER_GROUP("/api/depts", service::middleware::AuthMiddleware)
    RUVIA_ROUTES_BEGIN(DeptController)
    RUVIA_GET("/", list);
    RUVIA_GET("/tree", tree);
    RUVIA_GET("/:id", detail);
    RUVIA_POST("/", create, ruvia::json_body<CreateDeptBody>);
    RUVIA_PUT("/:id", update, ruvia::json_body<UpdateDeptBody>);
    RUVIA_DELETE("/:id", remove);
    RUVIA_ROUTES_END

  private:
    ruvia::task<ruvia::http_response> list(ruvia::context& c) {
        co_await service::middleware::requirePermission(c, "system:dept:query");
        const auto pageSize = service::common::parseInt64(c.request().query("page_size"));
        const auto [page, pageSizeValue, skip, keyword, paginated] =
            service::common::normalizePagination(
                service::common::parseInt64(c.request().query("page")), pageSize,
                c.request().query("keyword"));
        co_return c.json(service::common::ok<DeptPageResponse>(co_await deptService().list(
            c, page, pageSizeValue, skip, keyword, paginated, c.request().query("status"),
            service::common::parseInt64(c.request().query("parent_id")))));
    }

    ruvia::task<ruvia::http_response> tree(ruvia::context& c) {
        co_await service::middleware::requireAnyPermission(
            c, {"system:dept:query", "system:user:add", "system:user:edit"});
        co_return c.json(service::common::ok<DeptListResponse>(
            co_await deptService().getTree(c, c.request().query("status"))));
    }

    ruvia::task<ruvia::http_response> detail(ruvia::context& c) {
        co_await service::middleware::requirePermission(c, "system:dept:query");
        const auto id = service::common::parseInt64(c.request().param("id"));
        if (!id || *id <= 0)
            service::common::throwAppError(service::common::kValidationErrorCode, "id 必须是正整数",
                                           400);
        co_return c.json(
            service::common::ok<DeptDetailResponse>(co_await deptService().getById(c, *id)));
    }

    ruvia::task<ruvia::http_response> create(ruvia::context& c) {
        co_await service::middleware::requirePermission(c, "system:dept:add");
        co_await deptService().create(c, c.request().validated<CreateDeptBody>());
        co_return c.json(service::common::operation("创建成功"));
    }

    ruvia::task<ruvia::http_response> update(ruvia::context& c) {
        co_await service::middleware::requirePermission(c, "system:dept:edit");
        const auto id = service::common::parseInt64(c.request().param("id"));
        if (!id || *id <= 0)
            service::common::throwAppError(service::common::kValidationErrorCode, "id 必须是正整数",
                                           400);
        co_await deptService().update(c, *id, c.request().validated<UpdateDeptBody>());
        co_return c.json(service::common::operation("更新成功"));
    }

    ruvia::task<ruvia::http_response> remove(ruvia::context& c) {
        co_await service::middleware::requirePermission(c, "system:dept:delete");
        const auto id = service::common::parseInt64(c.request().param("id"));
        if (!id || *id <= 0)
            service::common::throwAppError(service::common::kValidationErrorCode, "id 必须是正整数",
                                           400);
        co_await deptService().remove(c, *id);
        co_return c.json(service::common::operation("删除成功"));
    }
};

} // namespace service::dept
