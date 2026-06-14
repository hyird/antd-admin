#pragma once

#include <ruvia/app/Task.h>
#include <ruvia/http/Context.h>
#include <ruvia/http/Controller.h>
#include <ruvia/http/HttpTypes.h>

#include "service/common/http.h"
#include "service/common/types.h"
#include "service/middleware/auth.h"
#include "service/middleware/permission.h"
#include "service/modules/system/dept/dept.schema.h"
#include "service/modules/system/dept/dept.service.h"

namespace service::dept {

class DeptController final : public ruvia::Controller<DeptController> {
  public:
    RUVIA_CONTROLLER_GROUP("/api/depts", service::middleware::AuthMiddleware)
    RUVIA_ROUTES_BEGIN
    RUVIA_GET("/", list);
    RUVIA_GET("/tree", tree);
    RUVIA_GET("/:id", detail);
    RUVIA_POST("/", create, CreateDeptValidator);
    RUVIA_PUT("/:id", update, UpdateDeptValidator);
    RUVIA_DELETE("/:id", remove);
    RUVIA_ROUTES_END

  private:
    ruvia::Task<ruvia::HttpResponse> list(ruvia::Context& c) {
        co_await service::middleware::requirePermission(c, "system:dept:query");
        auto pageSize = c.query("pageSize").toInt64();
        if (!pageSize)
            pageSize = c.query("page_size").toInt64();
        const auto [page, pageSizeValue, skip, keyword, paginated] =
            service::common::normalizePagination(c.query("page").toInt64(), pageSize,
                                                 c.query("keyword").toStringView());
        co_return c.json(service::common::ok<DeptPageResponse>(
            c, co_await deptService().list(c, page, pageSizeValue, skip, keyword, paginated,
                                           c.query("status").toStringView(),
                                           c.query("parent_id").toInt64())));
    }

    ruvia::Task<ruvia::HttpResponse> tree(ruvia::Context& c) {
        co_await service::middleware::requireAnyPermission(
            c, {"system:dept:query", "system:user:add", "system:user:edit"});
        co_return c.json(service::common::ok<DeptListResponse>(
            c, co_await deptService().getTree(c, c.query("status").toStringView())));
    }

    ruvia::Task<ruvia::HttpResponse> detail(ruvia::Context& c) {
        co_await service::middleware::requirePermission(c, "system:dept:query");
        const auto id = c.param("id").toInt64();
        if (!id || *id <= 0)
            service::common::throwAppError(service::common::kValidationErrorCode, "id 必须是正整数",
                                           400);
        co_return c.json(
            service::common::ok<DeptDetailResponse>(c, co_await deptService().getById(c, *id)));
    }

    ruvia::Task<ruvia::HttpResponse> create(ruvia::Context& c) {
        co_await service::middleware::requirePermission(c, "system:dept:add");
        co_await deptService().create(c, c.valid<CreateDeptBody>());
        co_return c.json(service::common::operation(c, "创建成功"));
    }

    ruvia::Task<ruvia::HttpResponse> update(ruvia::Context& c) {
        co_await service::middleware::requirePermission(c, "system:dept:edit");
        const auto id = c.param("id").toInt64();
        if (!id || *id <= 0)
            service::common::throwAppError(service::common::kValidationErrorCode, "id 必须是正整数",
                                           400);
        co_await deptService().update(c, *id, c.valid<UpdateDeptBody>());
        co_return c.json(service::common::operation(c, "更新成功"));
    }

    ruvia::Task<ruvia::HttpResponse> remove(ruvia::Context& c) {
        co_await service::middleware::requirePermission(c, "system:dept:delete");
        const auto id = c.param("id").toInt64();
        if (!id || *id <= 0)
            service::common::throwAppError(service::common::kValidationErrorCode, "id 必须是正整数",
                                           400);
        co_await deptService().remove(c, *id);
        co_return c.json(service::common::operation(c, "删除成功"));
    }
};

} // namespace service::dept
