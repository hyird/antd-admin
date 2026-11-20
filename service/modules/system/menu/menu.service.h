#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <task.h>
#include <db_executor.h>
#include <context.h>

#include <db_query.h>
#include <redis_executor.h>

#include "service/common/http.h"
#include "service/config/entities.h"
#include "service/middleware/permission.h"
#include "service/modules/system/menu/menu.error.h"
#include "service/modules/system/menu/menu.schema.h"
#include "service/modules/system/menu/menu.types.h"

namespace service::menu {

using ruvia::db_expr;

class MenuService {
  public:
    static MenuService& instance() {
        static MenuService svc;
        return svc;
    }

    template <typename Rows>
    static ruvia::boxed_array<MenuDto> flatFromRows(const Rows& rows) {
        return buildFlatList(rowsToRecords(rows));
    }

    ruvia::task<MenuPageDataDto> list(ruvia::context& c, std::int64_t page, std::int64_t pageSize,
                                      std::int64_t skip, const std::optional<std::string>& keyword,
                                      bool paginated, std::optional<std::string_view> status,
                                      std::optional<std::int64_t> parentId) {
        auto db = c.db();

        ruvia::db_query query;
        query.from(storage::Menu::table_name(), "m")
            .where(storage::Menu::column<"deleted_at">("m").is_null());
        if (keyword) {
            const auto like = "%" + ruvia::escape_like_pattern(*keyword) + "%";
            query.and_where(storage::Menu::column<"name">("m").like(like) ||
                            storage::Menu::column<"path">("m").like(like) ||
                            storage::Menu::column<"component">("m").like(like) ||
                            storage::Menu::column<"permission_code">("m").like(like));
        }
        if (status && !status->empty())
            query.and_where(storage::Menu::column<"status">("m").to_expr() == *status);
        if (parentId) {
            const auto parent = storage::Menu::column<"parent_id">("m").to_expr();
            query.and_where(*parentId <= 0 ? parent.is_null() : parent == *parentId);
        }

        query.select(db_expr::count());
        const auto countRows = co_await db.query(query);
        const auto total = countRows.front()[0].as<std::int64_t>().value_or(0);

        query.select({storage::Menu::column<"id">("m").to_expr(),
                      storage::Menu::column<"name">("m").to_expr(),
                      storage::Menu::column<"path">("m").to_expr(),
                      storage::Menu::column<"icon">("m").to_expr(),
                      storage::Menu::column<"parent_id">("m").to_expr(),
                      storage::Menu::column<"order">("m").to_expr(),
                      storage::Menu::column<"type">("m").to_expr(),
                      storage::Menu::column<"component">("m").to_expr(),
                      storage::Menu::column<"status">("m").to_expr(),
                      storage::Menu::column<"permission_code">("m").to_expr(),
                      storage::Menu::column<"is_default">("m").to_expr()});
        query.order_by(storage::Menu::column<"order">("m").to_expr());
        query.add_order_by(storage::Menu::column<"id">("m").to_expr());
        if (paginated)
            query.limit(pageSize).offset(skip);
        const auto rs = co_await db.query(query);

        MenuPageDataDto result;
        result.set<"total">(static_cast<ruvia::int64>(total))
            .set<"page">(static_cast<ruvia::int64>(page))
            .set<"pageSize">(static_cast<ruvia::int64>(pageSize))
            .set<"totalPages">(static_cast<ruvia::int64>(
                paginated && pageSize > 0 ? (total + pageSize - 1) / pageSize : 1
            ));

        auto& list = result.ensure<"list">();
        for (const auto& row : rs) {
            auto& item = list.emplace_back();
            fillMenuDto(item, rowToRecord(row));
        }
        co_return result;
    }

    ruvia::task<ruvia::boxed_array<MenuDto>> getTree(ruvia::context& c,
                                                     std::optional<std::string_view> status) {
        auto db = c.db();
        ruvia::db_query query;
        query
            .select({storage::Menu::column<"id">().to_expr(),
                     storage::Menu::column<"name">().to_expr(),
                     storage::Menu::column<"path">().to_expr(),
                     storage::Menu::column<"icon">().to_expr(),
                     storage::Menu::column<"parent_id">().to_expr(),
                     storage::Menu::column<"order">().to_expr(),
                     storage::Menu::column<"type">().to_expr(),
                     storage::Menu::column<"component">().to_expr(),
                     storage::Menu::column<"status">().to_expr(),
                     storage::Menu::column<"permission_code">().to_expr(),
                     storage::Menu::column<"is_default">().to_expr()})
            .from(storage::Menu::table_name())
            .where(storage::Menu::column<"deleted_at">().is_null());
        if (status && !status->empty())
            query.and_where(storage::Menu::column<"status">().to_expr() == *status);
        query.order_by(storage::Menu::column<"order">().to_expr());
        query.add_order_by(storage::Menu::column<"id">().to_expr());
        const auto rs = co_await db.query(query);
        co_return buildTree(rowsToRecords(rs));
    }

    ruvia::task<ruvia::boxed_array<MenuDto>> listEnabledFlat(ruvia::context& c) {
        auto db = c.db();
        const auto rs = co_await db.query(
            ruvia::db_query{}
                .select({storage::Menu::column<"id">().to_expr(),
                         storage::Menu::column<"name">().to_expr(),
                         storage::Menu::column<"path">().to_expr(),
                         storage::Menu::column<"icon">().to_expr(),
                         storage::Menu::column<"parent_id">().to_expr(),
                         storage::Menu::column<"order">().to_expr(),
                         storage::Menu::column<"type">().to_expr(),
                         storage::Menu::column<"component">().to_expr(),
                         storage::Menu::column<"status">().to_expr(),
                         storage::Menu::column<"permission_code">().to_expr(),
                         storage::Menu::column<"is_default">().to_expr()})
                .from(storage::Menu::table_name())
                .where(storage::Menu::column<"deleted_at">().to_expr().is_null() &&
                       storage::Menu::column<"status">().to_expr() == "enabled")
                .order_by(storage::Menu::column<"order">().to_expr())
                .add_order_by(storage::Menu::column<"id">().to_expr())
                .cache({.id_ = kEnabledMenuCacheId})
        );
        co_return flatFromRows(rs);
    }

    ruvia::task<MenuDto> getDetail(ruvia::context& c, std::int64_t id) {
        auto db = c.db();
        const auto rs =
            co_await db.query(ruvia::db_query{}
                                  .select({storage::Menu::column<"id">().to_expr(),
                                           storage::Menu::column<"name">().to_expr(),
                                           storage::Menu::column<"path">().to_expr(),
                                           storage::Menu::column<"icon">().to_expr(),
                                           storage::Menu::column<"parent_id">().to_expr(),
                                           storage::Menu::column<"order">().to_expr(),
                                           storage::Menu::column<"type">().to_expr(),
                                           storage::Menu::column<"component">().to_expr(),
                                           storage::Menu::column<"status">().to_expr(),
                                           storage::Menu::column<"permission_code">().to_expr(),
                                           storage::Menu::column<"is_default">().to_expr()})
                                  .from(storage::Menu::table_name())
                                  .where(storage::Menu::column<"id">().to_expr() == id &&
                                         storage::Menu::column<"deleted_at">().to_expr().is_null())
                                  .limit(1));
        if (rs.empty())
            service::common::throwAppError(MenuError::MENU_NOT_FOUND);

        MenuDto out;
        fillMenuDto(out, rowToRecord(rs.front()));
        co_return out;
    }

    ruvia::task<void> create(ruvia::context& c, const CreateMenuBody& body) {
        auto db = c.db();
        const std::string type =
            body.get<"type">() ? std::string(body.get<"type">()->view()) : "menu";

        std::string parentType;
        if (body.get<"parentId">()) {
            const auto prs = co_await db.query(
                ruvia::db_query{}
                    .select(storage::Menu::column<"type">().to_expr())
                    .from(storage::Menu::table_name())
                    .where(storage::Menu::column<"id">().to_expr() ==
                               static_cast<std::int64_t>(*body.get<"parentId">()) &&
                           storage::Menu::column<"deleted_at">().to_expr().is_null())
            );
            if (prs.empty())
                service::common::throwAppError(MenuError::MENU_PARENT_NOT_FOUND);
            parentType = std::string(prs.front()[0].value().value_or(""));
            checkParentChildType(parentType, type);
        }
        const bool isDefault =
            body.get<"isDefault">() && static_cast<bool>(*body.get<"isDefault">());
        if (isDefault && type != "page") {
            service::common::throwAppError(MenuError::DEFAULT_MUST_BE_PAGE);
        }

        auto tx = co_await db.begin_transaction();
        if (isDefault) {
            (void)co_await tx.execute(
                ruvia::db_query{}
                    .update(storage::Menu::table_name())
                    .set("is_default", db_expr::value(0))
                    .where(storage::Menu::column<"is_default">().to_expr() == 1)
            );
        }

        (void)co_await tx.execute(
            ruvia::db_query{}
                .insert_into(storage::Menu::table_name(),
                             {"name", "path", "icon", "component", "parent_id", "order", "type",
                              "status", "permission_code", "is_default", "created_at",
                              "updated_at"})
                .values(
                    {db_expr::value(ruvia::db_value{body.get<"name">().view()}),
                     db_expr::value(body.get<"path">() ? ruvia::db_value{body.get<"path">()->view()}
                                                       : ruvia::db_value{nullptr}),
                     db_expr::value(body.get<"icon">() ? ruvia::db_value{body.get<"icon">()->view()}
                                                       : ruvia::db_value{nullptr}),
                     db_expr::value(body.get<"component">()
                                        ? ruvia::db_value{body.get<"component">()->view()}
                                        : ruvia::db_value{nullptr}),
                     db_expr::value(
                         body.get<"parentId">()
                             ? ruvia::db_value{static_cast<std::int64_t>(*body.get<"parentId">())}
                             : ruvia::db_value{nullptr}
                     ),
                     db_expr::value(
                         ruvia::db_value{body.get<"sortOrder">()
                                             ? static_cast<std::int64_t>(*body.get<"sortOrder">())
                                             : 0}
                     ),
                     db_expr::value(ruvia::db_value{type}),
                     db_expr::value(ruvia::db_value{body.get<"status">()
                                                        ? body.get<"status">()->view()
                                                        : std::string_view{"enabled"}}),
                     db_expr::value(body.get<"permissionCode">()
                                        ? ruvia::db_value{body.get<"permissionCode">()->view()}
                                        : ruvia::db_value{nullptr}),
                     db_expr::value(ruvia::db_value{static_cast<std::int64_t>(isDefault ? 1 : 0)}),
                     db_expr::now(), db_expr::now()}
                )
        );
        co_await tx.commit();
        service::middleware::permissionService().clearAllCache();
        co_await clearEnabledMenuCache(db);
        co_return;
    }

    ruvia::task<void> update(ruvia::context& c, std::int64_t id, const UpdateMenuBody& body) {
        auto db = c.db();
        const auto rs =
            co_await db.query(ruvia::db_query{}
                                  .select({storage::Menu::column<"type">().to_expr(),
                                           storage::Menu::column<"parent_id">().to_expr()})
                                  .from(storage::Menu::table_name())
                                  .where(storage::Menu::column<"id">().to_expr() == id &&
                                         storage::Menu::column<"deleted_at">().to_expr().is_null())
                                  .limit(1));
        if (rs.empty())
            service::common::throwAppError(MenuError::MENU_NOT_FOUND);
        const std::string currentType(rs.front()[0].value().value_or(""));

        const auto type = body.get<"type">()
                              ? std::optional<std::string>(std::string(body.get<"type">()->view()))
                              : std::nullopt;
        const std::string newType = type.value_or(currentType);
        const auto newParentId =
            body.get<"parentId">()
                ? std::optional<std::int64_t>(static_cast<std::int64_t>(*body.get<"parentId">()))
                : std::nullopt;
        if (newParentId && *newParentId == id) {
            service::common::throwAppError(MenuError::MENU_PARENT_SELF);
        }
        if (newParentId && *newParentId > 0) {
            const auto prs = co_await db.query(
                ruvia::db_query{}
                    .select(storage::Menu::column<"type">().to_expr())
                    .from(storage::Menu::table_name())
                    .where(storage::Menu::column<"id">().to_expr() == *newParentId &&
                           storage::Menu::column<"deleted_at">().to_expr().is_null())
            );
            if (prs.empty())
                service::common::throwAppError(MenuError::MENU_PARENT_NOT_FOUND);
            checkParentChildType(std::string(prs.front()[0].value().value_or("")), newType);
            if (co_await isAncestorDescendant(c, id, *newParentId)) {
                service::common::throwAppError(MenuError::MENU_PARENT_IS_CHILD);
            }
        }
        if (body.get<"isDefault">() && static_cast<bool>(*body.get<"isDefault">()) &&
            newType != "page") {
            service::common::throwAppError(MenuError::DEFAULT_MUST_BE_PAGE);
        }

        bool clearOtherDefault = false;
        std::vector<ruvia::db_assignment> changes;
        auto append = [&](std::string_view col, ruvia::db_value value) {
            changes.emplace_back(col, db_expr::value(std::move(value)));
        };
        if (body.get<"name">())
            append("name", ruvia::db_value{body.get<"name">()->view()});
        if (body.get<"path">())
            append("path", ruvia::db_value{body.get<"path">()->view()});
        if (body.get<"icon">())
            append("icon", ruvia::db_value{body.get<"icon">()->view()});
        if (body.get<"component">())
            append("component", ruvia::db_value{body.get<"component">()->view()});
        if (newParentId)
            append("parent_id", ruvia::db_value{*newParentId});
        if (body.get<"sortOrder">())
            append("order", ruvia::db_value{static_cast<std::int64_t>(*body.get<"sortOrder">())});
        if (type)
            append("type", ruvia::db_value{*type});
        if (body.get<"status">())
            append("status", ruvia::db_value{body.get<"status">()->view()});
        if (body.get<"permissionCode">()) {
            append("permission_code", ruvia::db_value{body.get<"permissionCode">()->view()});
        }
        if (body.get<"isDefault">()) {
            const bool nextDefault = static_cast<bool>(*body.get<"isDefault">());
            clearOtherDefault = nextDefault;
            append("is_default", ruvia::db_value{static_cast<std::int64_t>(nextDefault ? 1 : 0)});
        }

        auto tx = co_await db.begin_transaction();
        if (clearOtherDefault) {
            (void)co_await tx.execute(
                ruvia::db_query{}
                    .update(storage::Menu::table_name())
                    .set("is_default", db_expr::value(0))
                    .where(storage::Menu::column<"is_default">().to_expr() == 1)
            );
        }
        if (!changes.empty()) {
            (void)co_await tx.execute(ruvia::db_query{}
                                          .update(storage::Menu::table_name())
                                          .set(changes)
                                          .set("updated_at", db_expr::now())
                                          .where(storage::Menu::column<"id">().to_expr() == id));
        }
        co_await tx.commit();
        service::middleware::permissionService().clearAllCache();
        co_await clearEnabledMenuCache(db);
        co_return;
    }

    ruvia::task<void> remove(ruvia::context& c, std::int64_t id) {
        auto db = c.db();
        const auto rs =
            co_await db.query(ruvia::db_query{}
                                  .select({storage::Menu::column<"id">().to_expr(),
                                           storage::Menu::column<"type">().to_expr()})
                                  .from(storage::Menu::table_name())
                                  .where(storage::Menu::column<"id">().to_expr() == id &&
                                         storage::Menu::column<"deleted_at">().to_expr().is_null())
                                  .limit(1));
        if (rs.empty())
            service::common::throwAppError(MenuError::MENU_NOT_FOUND);

        const std::string type(rs.front()[1].value().value_or(""));
        const auto children = storage::Menu::column<"parent_id">().to_expr() == id &&
                              storage::Menu::column<"deleted_at">().is_null();
        ruvia::db_query blockingChildren;
        blockingChildren.select(db_expr::count()).from(storage::Menu::table_name()).where(children);
        if (type == "page")
            blockingChildren.and_where(storage::Menu::column<"type">().to_expr() != "button");
        const auto count = co_await db.query(blockingChildren);
        if (count.front()[0].as<std::int64_t>().value_or(0) > 0) {
            service::common::throwAppError(MenuError::MENU_HAS_CHILDREN);
        }

        auto tx = co_await db.begin_transaction();
        if (type == "page") {
            const auto menuId = storage::RoleMenu::column<"menu_id">().to_expr();
            const auto pageButtons =
                children && storage::Menu::column<"type">().to_expr() == "button";
            ruvia::db_query matchingButton;
            matchingButton.select(db_expr::value(1))
                .from(storage::Menu::table_name())
                .where(pageButtons && storage::Menu::column<"id">().to_expr() == menuId);
            (void)co_await tx.execute(
                ruvia::db_query{}
                    .delete_from(storage::RoleMenu::table_name())
                    .where(menuId == id || ruvia::db_predicate::exists(matchingButton))
            );
            (void)co_await tx.execute(
                ruvia::db_query{}
                    .update(storage::Menu::table_name())
                    .set("deleted_at", db_expr::now())
                    .set("updated_at", db_expr::now())
                    .where(storage::Menu::column<"id">().to_expr() == id || pageButtons)
            );
        } else {
            (void)co_await tx.execute(
                ruvia::db_query{}
                    .delete_from(storage::RoleMenu::table_name())
                    .where(storage::RoleMenu::column<"menu_id">().to_expr() == id)
            );
            (void)co_await tx.execute(ruvia::db_query{}
                                          .update(storage::Menu::table_name())
                                          .set("deleted_at", db_expr::now())
                                          .set("updated_at", db_expr::now())
                                          .where(storage::Menu::column<"id">().to_expr() == id));
        }
        co_await tx.commit();
        service::middleware::permissionService().clearAllCache();
        co_await clearEnabledMenuCache(db);
        co_return;
    }

    ruvia::task<void> reorder(ruvia::context& c, const ReorderMenuBody& body) {
        if (body.get<"items">().empty())
            co_return;
        auto db = c.db();
        auto tx = co_await db.begin_transaction();
        for (const auto& item : body.get<"items">()) {
            (void)co_await tx.execute(
                ruvia::db_query{}
                    .update(storage::Menu::table_name())
                    .set("order", db_expr::value(ruvia::db_value{
                                      static_cast<std::int64_t>(item.get<"sortOrder">())}))
                    .set("updated_at", db_expr::now())
                    .where(storage::Menu::column<"id">().to_expr() ==
                           static_cast<std::int64_t>(item.get<"id">()))
            );
        }
        co_await tx.commit();
        service::middleware::permissionService().clearAllCache();
        co_await clearEnabledMenuCache(db);
        co_return;
    }

    ruvia::task<int> batchCreateButtons(ruvia::context& c, const BatchCreateMenuButtonsBody& body) {
        auto db = c.db();
        const auto parentId = static_cast<std::int64_t>(body.get<"parentId">());
        const auto prs = co_await db.query(
            ruvia::db_query{}
                .select(storage::Menu::column<"type">().to_expr())
                .from(storage::Menu::table_name())
                .where(storage::Menu::column<"id">().to_expr() == parentId &&
                       storage::Menu::column<"deleted_at">().to_expr().is_null())
        );
        if (prs.empty())
            service::common::throwAppError(MenuError::MENU_PARENT_NOT_FOUND);
        if (std::string(prs.front()[0].value().value_or("")) != "page") {
            service::common::throwAppError(MenuError::MENU_TYPE_INVALID);
        }

        auto tx = co_await db.begin_transaction();
        int created = 0;
        for (const auto& item : body.get<"items">()) {
            const std::string name(item.get<"name">().view());
            const std::string permissionCode(item.get<"permissionCode">().view());
            const auto exists = co_await tx.query(
                ruvia::db_query{}
                    .select(storage::Menu::column<"id">().to_expr())
                    .from(storage::Menu::table_name())
                    .where(storage::Menu::column<"parent_id">().to_expr() == parentId &&
                           storage::Menu::column<"type">().to_expr() == "button" &&
                           storage::Menu::column<"permission_code">().to_expr() == permissionCode &&
                           storage::Menu::column<"deleted_at">().to_expr().is_null())
                    .limit(1)
            );
            if (!exists.empty())
                continue;
            const auto maxRs = co_await tx.query(
                ruvia::db_query{}
                    .select(
                        db_expr::coalesce({db_expr::max(storage::Menu::column<"order">().to_expr()),
                                           db_expr::value(0)})
                    )
                    .from(storage::Menu::table_name())
                    .where(storage::Menu::column<"parent_id">().to_expr() == parentId)
            );
            const std::int64_t maxOrder =
                std::stoll(std::string(maxRs.front()[0].value().value_or("")));
            (void)co_await tx.execute(
                ruvia::db_query{}
                    .insert_into(storage::Menu::table_name(),
                                 {"name", "parent_id", "type", "status", "permission_code", "order",
                                  "created_at", "updated_at"})
                    .values({db_expr::value(ruvia::db_value{name}),
                             db_expr::value(ruvia::db_value{parentId}), db_expr::value("button"),
                             db_expr::value("enabled"),
                             db_expr::value(ruvia::db_value{permissionCode}),
                             db_expr::value(ruvia::db_value{maxOrder + 1}), db_expr::now(),
                             db_expr::now()})
            );
            ++created;
        }
        co_await tx.commit();
        service::middleware::permissionService().clearAllCache();
        co_await clearEnabledMenuCache(db);
        co_return created;
    }

  private:
    static constexpr std::string_view kEnabledMenuCacheId{"enabled-login-menus"};

    static ruvia::task<void> clearEnabledMenuCache(ruvia::db_executor& db) {
        try {
            co_await db.query_result_cache().remove(kEnabledMenuCacheId);
        } catch (const ruvia::redis_error&) {
            // The write already committed. Display data expires after the cache TTL.
        }
    }

    struct MenuRecord {
        std::int64_t id{0};
        std::string_view name;
        std::optional<std::string_view> path;
        std::optional<std::string_view> icon;
        std::optional<std::int64_t> parent_id;
        std::int64_t sort_order{0};
        std::string_view type;
        std::optional<std::string_view> component;
        std::string_view status;
        std::optional<std::string_view> permission_code;
        bool is_default{false};
    };

    MenuService() = default;

    static void checkParentChildType(const std::string& parentType, const std::string& childType) {
        if (parentType.empty())
            return;
        if (parentType == "button")
            service::common::throwAppError(MenuError::MENU_TYPE_INVALID);
        if (parentType == "page" && childType != "button") {
            service::common::throwAppError(MenuError::MENU_TYPE_INVALID);
        }
    }

    ruvia::task<bool> isAncestorDescendant(ruvia::context& c, std::int64_t ancestor,
                                           std::int64_t candidate) {
        auto db = c.db();
        const auto rs = co_await db.query(
            ruvia::db_query{}
                .select({storage::Menu::column<"id">().to_expr(),
                         storage::Menu::column<"parent_id">().to_expr()})
                .from(storage::Menu::table_name())
                .where(storage::Menu::column<"deleted_at">().to_expr().is_null())
        );
        std::unordered_map<std::int64_t, std::vector<std::int64_t>> children;
        for (const auto& row : rs) {
            if (!row[1].value().has_value())
                continue;
            const std::int64_t id = std::stoll(std::string(row[0].value().value_or("")));
            const std::int64_t parent = std::stoll(std::string(row[1].value().value_or("")));
            children[parent].push_back(id);
        }

        std::unordered_set<std::int64_t> seen{ancestor};
        std::vector<std::int64_t> stack{ancestor};
        while (!stack.empty()) {
            const auto current = stack.back();
            stack.pop_back();
            for (const auto child : children[current]) {
                if (child == candidate)
                    co_return true;
                if (seen.insert(child).second)
                    stack.push_back(child);
            }
        }
        co_return false;
    }

    template <typename Row>
    static MenuRecord rowToRecord(const Row& row) {
        MenuRecord item;
        item.id = std::stoll(std::string(row[0].value().value_or("")));
        item.name = row[1].value().value_or("");
        if (row[2].value().has_value())
            item.path = row[2].value().value_or("");
        if (row[3].value().has_value())
            item.icon = row[3].value().value_or("");
        if (row[4].value().has_value())
            item.parent_id = std::stoll(std::string(row[4].value().value_or("")));
        item.sort_order = std::stoll(std::string(row[5].value().value_or("")));
        item.type = row[6].value().value_or("");
        if (row[7].value().has_value())
            item.component = row[7].value().value_or("");
        item.status = row[8].value().value_or("");
        if (row[9].value().has_value())
            item.permission_code = row[9].value().value_or("");
        item.is_default = std::string(row[10].value().value_or("")) == "1";
        return item;
    }

    template <typename Rows>
    static std::vector<MenuRecord> rowsToRecords(const Rows& rows) {
        std::vector<MenuRecord> out;
        out.reserve(rows.size());
        for (const auto& row : rows)
            out.push_back(rowToRecord(row));
        return out;
    }

    static void fillMenuDto(MenuDto& item, const MenuRecord& record) {
        item.set<"id">(static_cast<ruvia::int64>(record.id))
            .set<"sortOrder">(static_cast<ruvia::int64>(record.sort_order))
            .set<"isDefault">(ruvia::bool_value{record.is_default});
        item.set<"name">(record.name);
        item.set<"type">(record.type);
        item.set<"status">(record.status);
        if (record.path) {
            item.set<"path">(*record.path);
            item.set<"fullPath">(*record.path);
        }
        if (record.icon)
            item.set<"icon">(*record.icon);
        if (record.parent_id)
            item.set<"parentId">(static_cast<ruvia::int64>(*record.parent_id));
        if (record.component)
            item.set<"component">(*record.component);
        if (record.permission_code)
            item.set<"permissionCode">(*record.permission_code);
    }

    static ruvia::boxed_array<MenuDto> buildFlatList(const std::vector<MenuRecord>& records) {
        ruvia::boxed_array<MenuDto> out;
        for (const auto& record : records) {
            auto& item = out.emplace_back();
            fillMenuDto(item, record);
        }
        return out;
    }

    static void
    appendNode(ruvia::boxed_array<MenuDto>& out, const MenuRecord& record,
               const std::unordered_map<std::int64_t, std::vector<const MenuRecord*>>& children,
               std::unordered_set<std::int64_t> path = {}) {
        if (!path.insert(record.id).second)
            return;

        auto& item = out.emplace_back();
        fillMenuDto(item, record);

        const auto it = children.find(record.id);
        if (it == children.end())
            return;
        auto& childList = item.ensure<"children">();
        for (const auto* child : it->second) {
            appendNode(childList, *child, children, path);
        }
    }

    static bool
    hasParentCycle(const MenuRecord& record,
                   const std::unordered_map<std::int64_t, const MenuRecord*>& recordsById) {
        std::unordered_set<std::int64_t> seen{record.id};
        auto parentId = record.parent_id;
        while (parentId) {
            const auto parent = recordsById.find(*parentId);
            if (parent == recordsById.end())
                return false;
            if (!seen.insert(*parentId).second)
                return true;
            parentId = parent->second->parent_id;
        }
        return false;
    }

    static ruvia::boxed_array<MenuDto> buildTree(const std::vector<MenuRecord>& records) {
        ruvia::boxed_array<MenuDto> out;
        std::unordered_map<std::int64_t, const MenuRecord*> recordsById;
        recordsById.reserve(records.size());
        for (const auto& record : records)
            recordsById[record.id] = &record;

        std::vector<const MenuRecord*> roots;
        std::unordered_map<std::int64_t, std::vector<const MenuRecord*>> children;
        for (const auto& record : records) {
            if (record.parent_id && recordsById.contains(*record.parent_id) &&
                !hasParentCycle(record, recordsById)) {
                children[*record.parent_id].push_back(&record);
            } else {
                roots.push_back(&record);
            }
        }

        for (const auto* root : roots)
            appendNode(out, *root, children);
        return out;
    }
};

inline MenuService& menuService() { return MenuService::instance(); }

} // namespace service::menu
