#pragma once

#include <cyra/app/Task.h>
#include <cyra/http/Context.h>
#include <cyra/http/Controller.h>
#include <cyra/http/HttpTypes.h>

#include "service/common/http.h"
#include "service/common/request.h"
#include "service/middleware/auth.h"
#include "service/middleware/permission.h"
#include "service/modules/system/role/role.schema.h"
#include "service/modules/system/role/role.service.h"

namespace service::modules::system::role {

class RoleController final : public cyra::Controller<RoleController> {
public:
    CYRA_CONTROLLER_GROUP("/api/roles", service::middleware::AuthMiddleware)
    CYRA_ROUTES_BEGIN
    CYRA_GET("/", list);
    CYRA_GET("/all", listAll);
    CYRA_GET("/:id", detail);
    CYRA_POST("/", create, CreateRoleValidator);
    CYRA_PUT("/:id", update, UpdateRoleValidator);
    CYRA_DELETE("/:id", remove);
    CYRA_ROUTES_END

private:
    cyra::Task<cyra::HttpResponse> list(cyra::Context& c) {
        co_await service::middleware::requirePermission(c, "system:role:query");
        RoleQuery q;
        static_cast<service::common::PageParams&>(q) = service::common::parsePageParams(c);
        q.status = service::common::getQuery(c, "status");
        co_return c.json(service::common::ok<RolePageResponse>(c, co_await roleService().list(c, q)));
    }

    cyra::Task<cyra::HttpResponse> listAll(cyra::Context& c) {
        co_await service::middleware::requireAnyPermission(c, {"system:user:add", "system:user:edit"});
        co_return c.json(service::common::ok<RoleOptionsResponse>(
            c,
            co_await roleService().listAllEnabled(c)));
    }

    cyra::Task<cyra::HttpResponse> detail(cyra::Context& c) {
        co_await service::middleware::requirePermission(c, "system:role:query");
        const auto id = service::common::parseIdParam(c);
        co_return c.json(service::common::ok<RoleDetailResponse>(
            c,
            co_await roleService().getById(c, id)));
    }

    cyra::Task<cyra::HttpResponse> create(cyra::Context& c) {
        co_await service::middleware::requirePermission(c, "system:role:add");
        co_await roleService().create(c, c.valid<CreateRoleBody>());
        co_return c.json(service::common::operation(c, "创建成功"));
    }

    cyra::Task<cyra::HttpResponse> update(cyra::Context& c) {
        co_await service::middleware::requirePermission(c, "system:role:edit");
        const auto id = service::common::parseIdParam(c);
        co_await roleService().update(c, id, c.valid<UpdateRoleBody>());
        co_return c.json(service::common::operation(c, "更新成功"));
    }

    cyra::Task<cyra::HttpResponse> remove(cyra::Context& c) {
        co_await service::middleware::requirePermission(c, "system:role:delete");
        const auto id = service::common::parseIdParam(c);
        co_await roleService().remove(c, id);
        co_return c.json(service::common::operation(c, "删除成功"));
    }
};

}  // namespace service::modules::system::role
