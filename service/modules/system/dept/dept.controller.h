#pragma once

#include <cyra/app/Task.h>
#include <cyra/http/Context.h>
#include <cyra/http/Controller.h>
#include <cyra/http/HttpTypes.h>

#include "service/common/http.h"
#include "service/common/types.h"
#include "service/middleware/auth.h"
#include "service/middleware/permission.h"
#include "service/modules/system/dept/dept.schema.h"
#include "service/modules/system/dept/dept.service.h"

namespace service::dept {

class DeptController final : public cyra::Controller<DeptController> {
  public:
    CYRA_CONTROLLER_GROUP("/api/depts", service::middleware::AuthMiddleware)
    CYRA_ROUTES_BEGIN
    CYRA_GET("/", list);
    CYRA_GET("/tree", tree);
    CYRA_GET("/:id", detail);
    CYRA_POST("/", create, CreateDeptValidator);
    CYRA_PUT("/:id", update, UpdateDeptValidator);
    CYRA_DELETE("/:id", remove);
    CYRA_ROUTES_END

  private:
    cyra::Task<cyra::HttpResponse> list(cyra::Context& c) {
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

    cyra::Task<cyra::HttpResponse> tree(cyra::Context& c) {
        co_await service::middleware::requireAnyPermission(
            c, {"system:dept:query", "system:user:add", "system:user:edit"});
        co_return c.json(service::common::ok<DeptListResponse>(
            c, co_await deptService().getTree(c, c.query("status").toStringView())));
    }

    cyra::Task<cyra::HttpResponse> detail(cyra::Context& c) {
        co_await service::middleware::requirePermission(c, "system:dept:query");
        const auto id = c.param("id").toInt64();
        if (!id || *id <= 0)
            service::common::throwAppError(service::common::kValidationErrorCode, "id 必须是正整数",
                                           400);
        co_return c.json(
            service::common::ok<DeptDetailResponse>(c, co_await deptService().getById(c, *id)));
    }

    cyra::Task<cyra::HttpResponse> create(cyra::Context& c) {
        co_await service::middleware::requirePermission(c, "system:dept:add");
        co_await deptService().create(c, c.valid<CreateDeptBody>());
        co_return c.json(service::common::operation(c, "创建成功"));
    }

    cyra::Task<cyra::HttpResponse> update(cyra::Context& c) {
        co_await service::middleware::requirePermission(c, "system:dept:edit");
        const auto id = c.param("id").toInt64();
        if (!id || *id <= 0)
            service::common::throwAppError(service::common::kValidationErrorCode, "id 必须是正整数",
                                           400);
        co_await deptService().update(c, *id, c.valid<UpdateDeptBody>());
        co_return c.json(service::common::operation(c, "更新成功"));
    }

    cyra::Task<cyra::HttpResponse> remove(cyra::Context& c) {
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
