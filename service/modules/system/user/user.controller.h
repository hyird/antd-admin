#pragma once

#include <cyra/app/Task.h>
#include <cyra/http/Context.h>
#include <cyra/http/Controller.h>
#include <cyra/http/HttpTypes.h>

#include "service/common/http.h"
#include "service/common/request.h"
#include "service/middleware/auth.h"
#include "service/middleware/permission.h"
#include "service/modules/system/user/user.schema.h"
#include "service/modules/system/user/user.service.h"

namespace service::modules::system::user {

class UserController final : public cyra::Controller<UserController> {
public:
    CYRA_CONTROLLER_GROUP("/api/users", service::middleware::AuthMiddleware)
    CYRA_ROUTES_BEGIN
    CYRA_GET("/", list);
    CYRA_GET("/options", options);
    CYRA_GET("/:id", detail);
    CYRA_POST("/", create, CreateUserValidator);
    CYRA_PUT("/:id", update, UpdateUserValidator);
    CYRA_DELETE("/:id", remove);
    CYRA_ROUTES_END

private:
    cyra::Task<cyra::HttpResponse> list(cyra::Context& c) {
        co_await service::middleware::requirePermission(c, "system:user:query");
        UserQuery q;
        static_cast<service::common::PageParams&>(q) = service::common::parsePageParams(c);
        q.status = service::common::getQuery(c, "status");
        q.dept_id = service::common::getQueryInt(c, "dept_id");
        co_return c.json(service::common::ok<UserPageResponse>(c, co_await userService().list(c, q)));
    }

    cyra::Task<cyra::HttpResponse> options(cyra::Context& c) {
        co_await service::middleware::requireAnyPermission(
            c,
            {"system:user:query", "system:user:add", "system:user:edit", "system:dept:query",
             "system:dept:add", "system:dept:edit"});
        co_return c.json(service::common::ok<UserOptionsResponse>(
            c,
            co_await userService().listOptions(c, service::common::getQuery(c, "keyword"))));
    }

    cyra::Task<cyra::HttpResponse> detail(cyra::Context& c) {
        co_await service::middleware::requirePermission(c, "system:user:query");
        const auto id = service::common::parseIdParam(c);
        co_return c.json(service::common::ok<UserDetailResponse>(
            c,
            co_await userService().getById(c, id)));
    }

    cyra::Task<cyra::HttpResponse> create(cyra::Context& c) {
        co_await service::middleware::requirePermission(c, "system:user:add");
        co_await userService().create(c, c.valid<CreateUserBody>());
        co_return c.json(service::common::operation(c, "创建成功"));
    }

    cyra::Task<cyra::HttpResponse> update(cyra::Context& c) {
        co_await service::middleware::requirePermission(c, "system:user:edit");
        const auto id = service::common::parseIdParam(c);
        co_await userService().update(c, id, c.valid<UpdateUserBody>());
        co_return c.json(service::common::operation(c, "更新成功"));
    }

    cyra::Task<cyra::HttpResponse> remove(cyra::Context& c) {
        co_await service::middleware::requirePermission(c, "system:user:delete");
        const auto id = service::common::parseIdParam(c);
        co_await userService().remove(c, id);
        co_return c.json(service::common::operation(c, "删除成功"));
    }
};

}  // namespace service::modules::system::user
