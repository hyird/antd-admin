#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <task.h>
#include <db_executor.h>
#include <context.h>

#include <db_query.h>
#include <db_repository.h>

#include "service/common/http.h"
#include "service/config/entities.h"
#include "service/common/types.h"
#include "service/middleware/permission.h"
#include "service/modules/system/role/role.error.h"
#include "service/modules/system/role/role.schema.h"
#include "service/modules/system/role/role.types.h"

namespace service::role {

using ruvia::db_expr;

class RoleService {
  public:
    static RoleService& instance() {
        static RoleService svc;
        return svc;
    }

    ruvia::task<RolePageDataDto> list(ruvia::context& c, std::int64_t page, std::int64_t pageSize,
                                      std::int64_t skip, const std::optional<std::string>& keyword,
                                      bool paginated, std::optional<std::string_view> status) {
        auto db = c.db();

        ruvia::db_query query;
        query.from(storage::Role::table_name(), "r")
            .where(storage::Role::column<"deleted_at">("r").is_null());
        if (keyword) {
            const auto like = "%" + ruvia::escape_like_pattern(*keyword) + "%";
            query.and_where(storage::Role::column<"name">("r").like(like) ||
                            storage::Role::column<"code">("r").like(like));
        }
        if (status && !status->empty())
            query.and_where(storage::Role::column<"status">("r").to_expr() == *status);

        query.select(db_expr::count());
        const auto countRows = co_await db.query(query);
        const auto total = countRows.front()[0].as<std::int64_t>().value_or(0);

        query.select({storage::Role::column<"id">("r").to_expr(),
                      storage::Role::column<"name">("r").to_expr(),
                      storage::Role::column<"code">("r").to_expr(),
                      storage::Role::column<"status">("r").to_expr()});
        query.order_by(storage::Role::column<"id">("r").to_expr());
        if (paginated)
            query.limit(pageSize).offset(skip);
        const auto rs = co_await db.query(query);

        RolePageDataDto result;
        result.set<"total">(static_cast<ruvia::int64>(total))
            .set<"page">(static_cast<ruvia::int64>(page))
            .set<"pageSize">(static_cast<ruvia::int64>(pageSize))
            .set<"totalPages">(static_cast<ruvia::int64>(
                paginated && pageSize > 0 ? (total + pageSize - 1) / pageSize : 1
            ));

        auto& list = result.ensure<"list">();
        for (const auto& row : rs) {
            auto& item = list.emplace_back();
            const auto id = std::stoll(std::string(row[0].value().value_or("")));
            const auto code = row[2].value().value_or("");
            item.set<"id">(static_cast<ruvia::int64>(id));
            item.set<"name">(row[1].value().value_or(""));
            item.set<"code">(code);
            item.set<"status">(row[3].value().value_or(""));
            const auto menuIds = code == service::common::kSuperAdminRoleCode
                                     ? co_await getAllMenuIds(c)
                                     : co_await getRoleMenuIds(c, id);
            setMenuIds(item, menuIds);
        }
        co_return result;
    }

    ruvia::task<RoleDetailDto> getById(ruvia::context& c, std::int64_t id) {
        auto db = c.db();
        const auto rs =
            co_await db.query(ruvia::db_query{}
                                  .select({storage::Role::column<"id">().to_expr(),
                                           storage::Role::column<"name">().to_expr(),
                                           storage::Role::column<"code">().to_expr(),
                                           storage::Role::column<"status">().to_expr()})
                                  .from(storage::Role::table_name())
                                  .where(storage::Role::column<"id">().to_expr() == id &&
                                         storage::Role::column<"deleted_at">().to_expr().is_null())
                                  .limit(1));
        if (rs.empty())
            service::common::throwAppError(RoleError::NOT_FOUND);

        const auto& row = rs.front();
        RoleDetailDto out;
        out.set<"id">(
            static_cast<ruvia::int64>(std::stoll(std::string(row[0].value().value_or(""))))
        );
        out.set<"name">(row[1].value().value_or(""));
        out.set<"code">(row[2].value().value_or(""));
        out.set<"status">(row[3].value().value_or(""));

        ruvia::array<ruvia::int64> menuIds;
        ruvia::array<RoleMenuDto> menus;
        const bool isSuperadmin =
            row[2].value().value_or("") == service::common::kSuperAdminRoleCode;
        const auto mrs =
            isSuperadmin
                ? co_await db.query(
                      ruvia::db_query{}
                          .select({storage::Menu::column<"id">("m").to_expr(),
                                   storage::Menu::column<"name">("m").to_expr(),
                                   storage::Menu::column<"type">("m").to_expr(),
                                   storage::Menu::column<"parent_id">("m").to_expr()})
                          .from(storage::Menu::table_name(), "m")
                          .where(storage::Menu::column<"deleted_at">("m").to_expr().is_null())
                          .order_by(storage::Menu::column<"order">("m").to_expr())
                          .add_order_by(storage::Menu::column<"id">("m").to_expr())
                  )
                : co_await db.query(
                      ruvia::db_query{}
                          .select({storage::Menu::column<"id">("m").to_expr(),
                                   storage::Menu::column<"name">("m").to_expr(),
                                   storage::Menu::column<"type">("m").to_expr(),
                                   storage::Menu::column<"parent_id">("m").to_expr()})
                          .from(storage::Menu::table_name(), "m")
                          .join(ruvia::db_join_type::inner, storage::RoleMenu::table_name(),
                                storage::Menu::column<"id">("m").to_expr() ==
                                    storage::RoleMenu::column<"menu_id">("rm").to_expr(),
                                "rm")
                          .where(storage::RoleMenu::column<"role_id">("rm").to_expr() == id &&
                                 storage::Menu::column<"deleted_at">("m").to_expr().is_null())
                          .order_by(storage::Menu::column<"order">("m").to_expr())
                          .add_order_by(storage::Menu::column<"id">("m").to_expr())
                  );
        for (const auto& mrow : mrs) {
            const auto mid = std::stoll(std::string(mrow[0].value().value_or("")));
            menuIds.emplace_back(static_cast<ruvia::int64>(mid));

            RoleMenuDto menu;
            menu.set<"id">(static_cast<ruvia::int64>(mid));
            menu.set<"name">(mrow[1].value().value_or(""));
            menu.set<"type">(mrow[2].value().value_or(""));
            if (mrow[3].value().has_value()) {
                menu.set<"parentId">(static_cast<ruvia::int64>(
                    std::stoll(std::string(mrow[3].value().value_or("")))
                ));
            }
            menus.push_back(std::move(menu));
        }
        out.set<"menuIds">(std::move(menuIds)).set<"menus">(std::move(menus));
        co_return out;
    }

    ruvia::task<ruvia::array<RoleOptionDto>> listAllEnabled(ruvia::context& c) {
        auto db = c.db();
        const auto rs =
            co_await db.query(ruvia::db_query{}
                                  .select({storage::Role::column<"id">().to_expr(),
                                           storage::Role::column<"name">().to_expr(),
                                           storage::Role::column<"code">().to_expr()})
                                  .from(storage::Role::table_name())
                                  .where(storage::Role::column<"status">().to_expr() == "enabled" &&
                                         storage::Role::column<"deleted_at">().to_expr().is_null())
                                  .order_by(storage::Role::column<"id">().to_expr()));
        ruvia::array<RoleOptionDto> out;
        for (const auto& row : rs) {
            auto& item = out.emplace_back();
            item.set<"id">(
                static_cast<ruvia::int64>(std::stoll(std::string(row[0].value().value_or(""))))
            );
            item.set<"name">(row[1].value().value_or(""));
            item.set<"code">(row[2].value().value_or(""));
        }
        co_return out;
    }

    ruvia::task<void> create(ruvia::context& c, const CreateRoleBody& body) {
        const std::string code(body.get<"code">().view());
        const std::string name(body.get<"name">().view());
        auto db = c.db();
        const auto exists =
            co_await db.query(ruvia::db_query{}
                                  .select(storage::Role::column<"id">().to_expr())
                                  .from(storage::Role::table_name())
                                  .where(storage::Role::column<"code">().to_expr() == code &&
                                         storage::Role::column<"deleted_at">().to_expr().is_null())
                                  .limit(1));
        if (!exists.empty())
            service::common::throwAppError(RoleError::CODE_EXISTS);

        const std::string status =
            body.get<"status">() ? std::string(body.get<"status">()->view()) : "enabled";
        auto tx = co_await db.begin_transaction();
        const auto rs = co_await tx.execute(
            ruvia::db_query{}
                .insert_into(storage::Role::table_name(),
                             {"name", "code", "status", "created_at", "updated_at"})
                .values({db_expr::value(ruvia::db_value{name}),
                         db_expr::value(ruvia::db_value{code}),
                         db_expr::value(ruvia::db_value{status}), db_expr::now(), db_expr::now()})
        );
        const std::int64_t roleId = static_cast<std::int64_t>(rs.last_insert_id().value_or(0));

        if (code != service::common::kSuperAdminRoleCode) {
            co_await syncRoleMenus(tx, roleId, body.get<"menuIds">());
        }
        co_await tx.commit();
        service::middleware::permissionService().clearAllCache();
        co_return;
    }

    ruvia::task<void> update(ruvia::context& c, std::int64_t id, const UpdateRoleBody& body) {
        auto db = c.db();
        const auto rs =
            co_await db.query(ruvia::db_query{}
                                  .select(storage::Role::column<"code">().to_expr())
                                  .from(storage::Role::table_name())
                                  .where(storage::Role::column<"id">().to_expr() == id &&
                                         storage::Role::column<"deleted_at">().to_expr().is_null())
                                  .limit(1));
        if (rs.empty())
            service::common::throwAppError(RoleError::NOT_FOUND);
        const std::string currentCode(rs.front()[0].value().value_or(""));

        const auto code = body.get<"code">()
                              ? std::optional<std::string>(std::string(body.get<"code">()->view()))
                              : std::nullopt;
        if (currentCode == service::common::kSuperAdminRoleCode && code &&
            *code != service::common::kSuperAdminRoleCode) {
            service::common::throwAppError(RoleError::SUPERADMIN_CANNOT_MODIFY);
        }

        std::vector<ruvia::db_assignment> changes;
        auto append = [&](std::string_view col, ruvia::db_value value) {
            changes.emplace_back(col, db_expr::value(std::move(value)));
        };
        if (body.get<"name">())
            append("name", ruvia::db_value{body.get<"name">()->view()});
        if (code && *code != currentCode) {
            const auto existsRs = co_await db.query(
                ruvia::db_query{}
                    .select(storage::Role::column<"id">().to_expr())
                    .from(storage::Role::table_name())
                    .where(storage::Role::column<"code">().to_expr() == *code &&
                           storage::Role::column<"id">().to_expr() != id &&
                           storage::Role::column<"deleted_at">().to_expr().is_null())
                    .limit(1)
            );
            if (!existsRs.empty())
                service::common::throwAppError(RoleError::CODE_EXISTS);
            append("code", ruvia::db_value{*code});
        }
        if (body.get<"status">())
            append("status", ruvia::db_value{body.get<"status">()->view()});

        auto tx = co_await db.begin_transaction();
        if (!changes.empty()) {
            (void)co_await tx.execute(ruvia::db_query{}
                                          .update(storage::Role::table_name())
                                          .set(changes)
                                          .set("updated_at", db_expr::now())
                                          .where(storage::Role::column<"id">().to_expr() == id));
        }

        const std::string effectiveCode = code.value_or(currentCode);
        if (body.get<"menuIds">()) {
            if (effectiveCode == service::common::kSuperAdminRoleCode) {
                (void)co_await tx.execute(
                    ruvia::db_query{}
                        .delete_from(storage::RoleMenu::table_name())
                        .where(storage::RoleMenu::column<"role_id">().to_expr() == id)
                );
            } else {
                co_await syncRoleMenus(tx, id, body.get<"menuIds">());
            }
        }
        co_await tx.commit();
        service::middleware::permissionService().clearAllCache();
        co_return;
    }

    ruvia::task<void> remove(ruvia::context& c, std::int64_t id) {
        auto db = c.db();
        const auto rs =
            co_await db.query(ruvia::db_query{}
                                  .select(storage::Role::column<"code">().to_expr())
                                  .from(storage::Role::table_name())
                                  .where(storage::Role::column<"id">().to_expr() == id &&
                                         storage::Role::column<"deleted_at">().to_expr().is_null())
                                  .limit(1));
        if (rs.empty())
            service::common::throwAppError(RoleError::NOT_FOUND);
        const std::string code(rs.front()[0].value().value_or(""));
        if (code == service::common::kSuperAdminRoleCode) {
            service::common::throwAppError(RoleError::SUPERADMIN_CANNOT_DELETE);
        }

        const auto userRs =
            co_await db.query(ruvia::db_query{}
                                  .select(db_expr::count())
                                  .from(storage::UserRole::table_name())
                                  .where(storage::UserRole::column<"role_id">().to_expr() == id));
        const std::int64_t userCount =
            std::stoll(std::string(userRs.front()[0].value().value_or("")));
        if (userCount > 0)
            service::common::throwAppError(RoleError::HAS_USERS);

        (void)co_await db.execute(ruvia::db_query{}
                                      .update(storage::Role::table_name())
                                      .set("deleted_at", db_expr::now())
                                      .where(storage::Role::column<"id">().to_expr() == id));
        service::middleware::permissionService().clearAllCache();
        co_return;
    }

    ruvia::task<std::vector<std::int64_t>> getRoleMenuIds(ruvia::context& c, std::int64_t roleId) {
        auto db = c.db();
        const auto rs = co_await db.query(
            ruvia::db_query{}
                .select(storage::RoleMenu::column<"menu_id">().to_expr())
                .from(storage::RoleMenu::table_name())
                .where(storage::RoleMenu::column<"role_id">().to_expr() == roleId)
        );
        std::vector<std::int64_t> ids;
        ids.reserve(rs.size());
        for (const auto& row : rs) {
            ids.push_back(std::stoll(std::string(row[0].value().value_or(""))));
        }
        co_return ids;
    }

  private:
    RoleService() = default;

    ruvia::task<std::vector<std::int64_t>> getAllMenuIds(ruvia::context& c) {
        auto db = c.db();
        const auto rs =
            co_await db.query(ruvia::db_query{}
                                  .select(storage::Menu::column<"id">().to_expr())
                                  .from(storage::Menu::table_name())
                                  .where(storage::Menu::column<"deleted_at">().to_expr().is_null())
                                  .order_by(storage::Menu::column<"order">().to_expr())
                                  .add_order_by(storage::Menu::column<"id">().to_expr()));
        std::vector<std::int64_t> ids;
        ids.reserve(rs.size());
        for (const auto& row : rs) {
            ids.push_back(std::stoll(std::string(row[0].value().value_or(""))));
        }
        co_return ids;
    }

    static void setMenuIds(RoleItemDto& item, const std::vector<std::int64_t>& values) {
        ruvia::array<ruvia::int64> menuIds;
        menuIds.reserve(values.size());
        for (const auto value : values) {
            menuIds.emplace_back(static_cast<ruvia::int64>(value));
        }
        item.set<"menuIds">(std::move(menuIds));
    }

    ruvia::task<void> syncRoleMenus(ruvia::db_transaction& tx, std::int64_t roleId,
                                    const std::optional<ruvia::array<ruvia::int64>>& menuIds) {
        (void)co_await tx.execute(
            ruvia::db_query{}
                .delete_from(storage::RoleMenu::table_name())
                .where(storage::RoleMenu::column<"role_id">().to_expr() == roleId)
        );
        if (!menuIds)
            co_return;
        for (const auto menuId : *menuIds) {
            storage::RoleMenu relation;
            relation.set<"role_id">(roleId);
            relation.set<"menu_id">(static_cast<std::int64_t>(menuId));
            const std::array<ruvia::borrowed_text, 1> duplicateKey{"role_id"};
            (void)co_await tx.repository<storage::RoleMenu>().upsert(
                relation, {.update_columns_ = duplicateKey, .any_unique_key_ = true}
            );
        }
        co_return;
    }
};

inline RoleService& roleService() { return RoleService::instance(); }

} // namespace service::role
