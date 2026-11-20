#pragma once

#include <task.h>
#include <context.h>
#include <controller.h>

#include "service/common/http.h"
#include "service/common/types.h"
#include "service/middleware/auth.h"
#include "service/middleware/permission.h"
#include "service/modules/system/menu/menu.schema.h"
#include "service/modules/system/menu/menu.service.h"

namespace service::menu {

class MenuController final {
  public:
    RUVIA_CONTROLLER_GROUP("/api/menus", service::middleware::AuthMiddleware)
    RUVIA_ROUTES_BEGIN(MenuController)
    RUVIA_GET("/", list);
    RUVIA_GET("/tree", tree);
    RUVIA_GET("/:id", detail);
    RUVIA_POST("/", create, ruvia::json_body<CreateMenuBody>);
    RUVIA_PUT("/:id", update, ruvia::json_body<UpdateMenuBody>);
    RUVIA_POST("/reorder", reorder, ruvia::json_body<ReorderMenuBody>);
    RUVIA_POST("/batch-buttons", batchButtons, ruvia::json_body<BatchCreateMenuButtonsBody>);
    RUVIA_DELETE("/:id", remove);
    RUVIA_ROUTES_END

  private:
    ruvia::task<ruvia::http_response> list(ruvia::context& c) {
        co_await service::middleware::requirePermission(c, "system:menu:query");
        const auto pageSize = service::common::parseInt64(c.request().query("page_size"));
        const auto [page, pageSizeValue, skip, keyword, paginated] =
            service::common::normalizePagination(
                service::common::parseInt64(c.request().query("page")), pageSize,
                c.request().query("keyword")
            );
        co_return c.json(service::common::ok<MenuPageResponse>(co_await menuService().list(
            c, page, pageSizeValue, skip, keyword, paginated, c.request().query("status"),
            service::common::parseInt64(c.request().query("parent_id"))
        )));
    }

    ruvia::task<ruvia::http_response> tree(ruvia::context& c) {
        co_await service::middleware::requireAnyPermission(
            c, {"system:menu:query", "system:role:perm"}
        );
        co_return c.json(service::common::ok<MenuListResponse>(
            co_await menuService().getTree(c, c.request().query("status"))
        ));
    }

    ruvia::task<ruvia::http_response> detail(ruvia::context& c) {
        co_await service::middleware::requirePermission(c, "system:menu:query");
        const auto id = service::common::parseInt64(c.request().param("id"));
        if (!id || *id <= 0)
            service::common::throwAppError(service::common::kValidationErrorCode, "id 必须是正整数",
                                           400);
        co_return c.json(
            service::common::ok<MenuDetailResponse>(co_await menuService().getDetail(c, *id))
        );
    }

    ruvia::task<ruvia::http_response> create(ruvia::context& c) {
        co_await service::middleware::requirePermission(c, "system:menu:add");
        co_await menuService().create(c, c.request().validated<CreateMenuBody>());
        co_return c.json(service::common::operation("创建成功"));
    }

    ruvia::task<ruvia::http_response> update(ruvia::context& c) {
        co_await service::middleware::requirePermission(c, "system:menu:edit");
        const auto id = service::common::parseInt64(c.request().param("id"));
        if (!id || *id <= 0)
            service::common::throwAppError(service::common::kValidationErrorCode, "id 必须是正整数",
                                           400);
        co_await menuService().update(c, *id, c.request().validated<UpdateMenuBody>());
        co_return c.json(service::common::operation("更新成功"));
    }

    ruvia::task<ruvia::http_response> reorder(ruvia::context& c) {
        co_await service::middleware::requirePermission(c, "system:menu:edit");
        co_await menuService().reorder(c, c.request().validated<ReorderMenuBody>());
        co_return c.json(service::common::operation("更新成功"));
    }

    ruvia::task<ruvia::http_response> batchButtons(ruvia::context& c) {
        co_await service::middleware::requirePermission(c, "system:menu:add");
        const int created = co_await menuService().batchCreateButtons(
            c, c.request().validated<BatchCreateMenuButtonsBody>()
        );
        co_return c.json(service::common::count(created, "创建成功"));
    }

    ruvia::task<ruvia::http_response> remove(ruvia::context& c) {
        co_await service::middleware::requirePermission(c, "system:menu:delete");
        const auto id = service::common::parseInt64(c.request().param("id"));
        if (!id || *id <= 0)
            service::common::throwAppError(service::common::kValidationErrorCode, "id 必须是正整数",
                                           400);
        co_await menuService().remove(c, *id);
        co_return c.json(service::common::operation("删除成功"));
    }
};

} // namespace service::menu
