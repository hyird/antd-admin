#pragma once

#include <ruvia/core/Task.h>
#include <ruvia/web/Context.h>
#include <ruvia/web/Controller.h>

#include "service/common/http.h"
#include "service/common/types.h"
#include "service/middleware/auth.h"
#include "service/middleware/permission.h"
#include "service/modules/system/menu/menu.schema.h"
#include "service/modules/system/menu/menu.service.h"

namespace service::menu {

class MenuController final : public ruvia::Controller<MenuController> {
  public:
    RUVIA_CONTROLLER_GROUP("/api/menus", service::middleware::AuthMiddleware)
    RUVIA_ROUTES_BEGIN
    RUVIA_GET("/", list);
    RUVIA_GET("/tree", tree);
    RUVIA_GET("/:id", detail);
    RUVIA_POST("/", create, CreateMenuValidator);
    RUVIA_PUT("/:id", update, UpdateMenuValidator);
    RUVIA_POST("/reorder", reorder, ReorderMenuValidator);
    RUVIA_POST("/batch-buttons", batchButtons, BatchCreateMenuButtonsValidator);
    RUVIA_DELETE("/:id", remove);
    RUVIA_ROUTES_END

  private:
    ruvia::Task<ruvia::HttpResponse> list(ruvia::Context& c) {
        co_await service::middleware::requirePermission(c, "system:menu:query");
        auto pageSize = service::common::parseInt64(c.req().query("pageSize"));
        if (!pageSize)
            pageSize = service::common::parseInt64(c.req().query("page_size"));
        const auto [page, pageSizeValue, skip, keyword, paginated] =
            service::common::normalizePagination(service::common::parseInt64(c.req().query("page")), pageSize,
                                                 c.req().query("keyword"));
        co_return c.json(service::common::ok<MenuPageResponse>(
            c, co_await menuService().list(c, page, pageSizeValue, skip, keyword, paginated,
                                           c.req().query("status"),
                                           service::common::parseInt64(c.req().query("parent_id")))));
    }

    ruvia::Task<ruvia::HttpResponse> tree(ruvia::Context& c) {
        co_await service::middleware::requireAnyPermission(
            c, {"system:menu:query", "system:role:perm"});
        co_return c.json(service::common::ok<MenuListResponse>(
            c, co_await menuService().getTree(c, c.req().query("status"))));
    }

    ruvia::Task<ruvia::HttpResponse> detail(ruvia::Context& c) {
        co_await service::middleware::requirePermission(c, "system:menu:query");
        const auto id = service::common::parseInt64(c.req().param("id"));
        if (!id || *id <= 0)
            service::common::throwAppError(service::common::kValidationErrorCode, "id 必须是正整数",
                                           400);
        co_return c.json(
            service::common::ok<MenuDetailResponse>(c, co_await menuService().getDetail(c, *id)));
    }

    ruvia::Task<ruvia::HttpResponse> create(ruvia::Context& c) {
        co_await service::middleware::requirePermission(c, "system:menu:add");
        co_await menuService().create(c, c.req().valid<CreateMenuBody>());
        co_return c.json(service::common::operation(c, "创建成功"));
    }

    ruvia::Task<ruvia::HttpResponse> update(ruvia::Context& c) {
        co_await service::middleware::requirePermission(c, "system:menu:edit");
        const auto id = service::common::parseInt64(c.req().param("id"));
        if (!id || *id <= 0)
            service::common::throwAppError(service::common::kValidationErrorCode, "id 必须是正整数",
                                           400);
        co_await menuService().update(c, *id, c.req().valid<UpdateMenuBody>());
        co_return c.json(service::common::operation(c, "更新成功"));
    }

    ruvia::Task<ruvia::HttpResponse> reorder(ruvia::Context& c) {
        co_await service::middleware::requirePermission(c, "system:menu:edit");
        co_await menuService().reorder(c, c.req().valid<ReorderMenuBody>());
        co_return c.json(service::common::operation(c, "更新成功"));
    }

    ruvia::Task<ruvia::HttpResponse> batchButtons(ruvia::Context& c) {
        co_await service::middleware::requirePermission(c, "system:menu:add");
        const int created =
            co_await menuService().batchCreateButtons(c, c.req().valid<BatchCreateMenuButtonsBody>());
        co_return c.json(service::common::count(c, created, "创建成功"));
    }

    ruvia::Task<ruvia::HttpResponse> remove(ruvia::Context& c) {
        co_await service::middleware::requirePermission(c, "system:menu:delete");
        const auto id = service::common::parseInt64(c.req().param("id"));
        if (!id || *id <= 0)
            service::common::throwAppError(service::common::kValidationErrorCode, "id 必须是正整数",
                                           400);
        co_await menuService().remove(c, *id);
        co_return c.json(service::common::operation(c, "删除成功"));
    }
};

} // namespace service::menu
