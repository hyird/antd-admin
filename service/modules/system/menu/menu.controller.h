#pragma once

#include <cyra/app/Task.h>
#include <cyra/http/Context.h>
#include <cyra/http/Controller.h>
#include <cyra/http/HttpTypes.h>

#include "service/common/http.h"
#include "service/common/types.h"
#include "service/middleware/auth.h"
#include "service/middleware/permission.h"
#include "service/modules/system/menu/menu.schema.h"
#include "service/modules/system/menu/menu.service.h"

namespace service::menu {

class MenuController final : public cyra::Controller<MenuController> {
  public:
    CYRA_CONTROLLER_GROUP("/api/menus", service::middleware::AuthMiddleware)
    CYRA_ROUTES_BEGIN
    CYRA_GET("/", list);
    CYRA_GET("/tree", tree);
    CYRA_GET("/:id", detail);
    CYRA_POST("/", create, CreateMenuValidator);
    CYRA_PUT("/:id", update, UpdateMenuValidator);
    CYRA_POST("/reorder", reorder, ReorderMenuValidator);
    CYRA_POST("/batch-buttons", batchButtons, BatchCreateMenuButtonsValidator);
    CYRA_DELETE("/:id", remove);
    CYRA_ROUTES_END

  private:
    cyra::Task<cyra::HttpResponse> list(cyra::Context& c) {
        co_await service::middleware::requirePermission(c, "system:menu:query");
        auto pageSize = c.query("pageSize").toInt64();
        if (!pageSize)
            pageSize = c.query("page_size").toInt64();
        const auto [page, pageSizeValue, skip, keyword, paginated] =
            service::common::normalizePagination(c.query("page").toInt64(), pageSize,
                                                 c.query("keyword").toStringView());
        co_return c.json(service::common::ok<MenuPageResponse>(
            c, co_await menuService().list(c, page, pageSizeValue, skip, keyword, paginated,
                                           c.query("status").toStringView(),
                                           c.query("parent_id").toInt64())));
    }

    cyra::Task<cyra::HttpResponse> tree(cyra::Context& c) {
        co_await service::middleware::requireAnyPermission(
            c, {"system:menu:query", "system:role:perm"});
        co_return c.json(service::common::ok<MenuListResponse>(
            c, co_await menuService().getTree(c, c.query("status").toStringView())));
    }

    cyra::Task<cyra::HttpResponse> detail(cyra::Context& c) {
        co_await service::middleware::requirePermission(c, "system:menu:query");
        const auto id = c.param("id").toInt64();
        if (!id || *id <= 0)
            service::common::throwAppError(service::common::kValidationErrorCode, "id 必须是正整数",
                                           400);
        co_return c.json(
            service::common::ok<MenuDetailResponse>(c, co_await menuService().getDetail(c, *id)));
    }

    cyra::Task<cyra::HttpResponse> create(cyra::Context& c) {
        co_await service::middleware::requirePermission(c, "system:menu:add");
        co_await menuService().create(c, c.valid<CreateMenuBody>());
        co_return c.json(service::common::operation(c, "创建成功"));
    }

    cyra::Task<cyra::HttpResponse> update(cyra::Context& c) {
        co_await service::middleware::requirePermission(c, "system:menu:edit");
        const auto id = c.param("id").toInt64();
        if (!id || *id <= 0)
            service::common::throwAppError(service::common::kValidationErrorCode, "id 必须是正整数",
                                           400);
        co_await menuService().update(c, *id, c.valid<UpdateMenuBody>());
        co_return c.json(service::common::operation(c, "更新成功"));
    }

    cyra::Task<cyra::HttpResponse> reorder(cyra::Context& c) {
        co_await service::middleware::requirePermission(c, "system:menu:edit");
        co_await menuService().reorder(c, c.valid<ReorderMenuBody>());
        co_return c.json(service::common::operation(c, "更新成功"));
    }

    cyra::Task<cyra::HttpResponse> batchButtons(cyra::Context& c) {
        co_await service::middleware::requirePermission(c, "system:menu:add");
        const int created =
            co_await menuService().batchCreateButtons(c, c.valid<BatchCreateMenuButtonsBody>());
        co_return c.json(service::common::count(c, created, "创建成功"));
    }

    cyra::Task<cyra::HttpResponse> remove(cyra::Context& c) {
        co_await service::middleware::requirePermission(c, "system:menu:delete");
        const auto id = c.param("id").toInt64();
        if (!id || *id <= 0)
            service::common::throwAppError(service::common::kValidationErrorCode, "id 必须是正整数",
                                           400);
        co_await menuService().remove(c, *id);
        co_return c.json(service::common::operation(c, "删除成功"));
    }
};

} // namespace service::menu
