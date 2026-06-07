#pragma once

#include <cyra/app/Task.h>
#include <cyra/http/Context.h>
#include <cyra/http/Controller.h>
#include <cyra/http/HttpTypes.h>

#include "service/common/http.h"
#include "service/common/request.h"
#include "service/middleware/auth.h"
#include "service/middleware/permission.h"
#include "service/modules/system/dept/dept.schema.h"
#include "service/modules/system/dept/dept.service.h"

namespace service::modules::system::dept {

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
        co_return c.json(service::common::ok<DeptListResponse>(
            c,
            co_await deptService().listAll(c, service::common::getQuery(c, "keyword"))));
    }

    cyra::Task<cyra::HttpResponse> tree(cyra::Context& c) {
        co_await service::middleware::requireAnyPermission(
            c, {"system:dept:query", "system:user:add", "system:user:edit"});
        co_return c.json(service::common::ok<DeptListResponse>(
            c,
            co_await deptService().getTree(c, service::common::getQuery(c, "status"))));
    }

    cyra::Task<cyra::HttpResponse> detail(cyra::Context& c) {
        co_await service::middleware::requirePermission(c, "system:dept:query");
        const auto id = service::common::parseIdParam(c);
        co_return c.json(service::common::ok<DeptDetailResponse>(
            c,
            co_await deptService().getById(c, id)));
    }

    cyra::Task<cyra::HttpResponse> create(cyra::Context& c) {
        co_await service::middleware::requirePermission(c, "system:dept:add");
        co_await deptService().create(c, c.valid<CreateDeptBody>());
        co_return c.json(service::common::operation(c, "创建成功"));
    }

    cyra::Task<cyra::HttpResponse> update(cyra::Context& c) {
        co_await service::middleware::requirePermission(c, "system:dept:edit");
        const auto id = service::common::parseIdParam(c);
        co_await deptService().update(c, id, c.valid<UpdateDeptBody>());
        co_return c.json(service::common::operation(c, "更新成功"));
    }

    cyra::Task<cyra::HttpResponse> remove(cyra::Context& c) {
        co_await service::middleware::requirePermission(c, "system:dept:delete");
        const auto id = service::common::parseIdParam(c);
        co_await deptService().remove(c, id);
        co_return c.json(service::common::operation(c, "删除成功"));
    }
};

}  // namespace service::modules::system::dept
