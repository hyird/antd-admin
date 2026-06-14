#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <ruvia/app/Task.h>
#include <ruvia/db/Db.h>
#include <ruvia/http/Context.h>

#include "service/common/http.h"
#include "service/common/types.h"
#include "service/middleware/permission.h"
#include "service/modules/system/role/role.error.h"
#include "service/modules/system/role/role.types.h"

namespace service::role {

class RoleService {
  public:
    static RoleService& instance() {
        static RoleService svc;
        return svc;
    }

    ruvia::Task<RolePageDataDto> list(ruvia::Context& c, std::int64_t page, std::int64_t pageSize,
                                     std::int64_t skip, const std::optional<std::string>& keyword,
                                     bool paginated, std::optional<std::string_view> status) {
        auto db = c.db();

        std::string where = " FROM sys_role r WHERE r.deleted_at IS NULL";
        std::vector<ruvia::DbValue> params;
        if (keyword) {
            where += " AND (r.name LIKE ? OR r.code LIKE ?)";
            const std::string like = "%" + *keyword + "%";
            params.emplace_back(like);
            params.emplace_back(like);
        }
        if (status && !status->empty()) {
            where += " AND r.status = ?";
            params.emplace_back(std::string(*status));
        }

        const auto countRs = co_await db.query("SELECT COUNT(*)" + where, params);
        const std::int64_t total =
            countRs.rows().empty() ? 0 : std::stoll(std::string(countRs.rows().front()[0].text()));

        std::string sql = "SELECT r.id, r.name, r.code, r.status" + where + " ORDER BY r.id ASC";
        if (paginated) {
            sql += " LIMIT " + std::to_string(pageSize) + " OFFSET " + std::to_string(skip);
        }
        const auto rs = co_await db.query(sql, params);

        RolePageDataDto result(c);
        result.total(static_cast<ruvia::Int64>(total))
            .page(static_cast<ruvia::Int64>(page))
            .pageSize(static_cast<ruvia::Int64>(pageSize))
            .totalPages(static_cast<ruvia::Int64>(
                paginated && pageSize > 0 ? (total + pageSize - 1) / pageSize : 1));

        auto& list = result.list().ensure();
        for (const auto& row : rs.rows()) {
            auto& item = list.emplace(c);
            const auto id = std::stoll(std::string(row[0].text()));
            const auto code = row[2].text();
            item.id(static_cast<ruvia::Int64>(id));
            item.name().assignView(row[1].text());
            item.code().assignView(code);
            item.status().assignView(row[3].text());
            const auto menuIds = code == service::common::kSuperAdminRoleCode
                                     ? co_await getAllMenuIds(c)
                                     : co_await getRoleMenuIds(c, id);
            setMenuIds(c, item, menuIds);
        }
        co_return result;
    }

    ruvia::Task<RoleDetailDto> getById(ruvia::Context& c, std::int64_t id) {
        auto db = c.db();
        const auto rs = co_await db.query("SELECT id, name, code, status FROM sys_role WHERE id = "
                                          "? AND deleted_at IS NULL LIMIT 1",
                                          {ruvia::DbValue{id}});
        if (rs.rows().empty())
            service::common::throwAppError(RoleError::NOT_FOUND);

        const auto& row = rs.rows().front();
        RoleDetailDto out(c);
        out.id(static_cast<ruvia::Int64>(std::stoll(std::string(row[0].text()))));
        out.name().assignView(row[1].text());
        out.code().assignView(row[2].text());
        out.status().assignView(row[3].text());

        ruvia::Array<ruvia::Int64> menuIds(c.allocator<ruvia::Int64>());
        ruvia::Array<RoleMenuDto> menus(c.allocator<RoleMenuDto>());
        const bool isSuperadmin = row[2].text() == service::common::kSuperAdminRoleCode;
        const auto mrs =
            isSuperadmin
                ? co_await db.query("SELECT m.id, m.name, m.type, m.parent_id FROM sys_menu m "
                                    "WHERE m.deleted_at IS NULL "
                                    "ORDER BY m.`order` ASC, m.id ASC")
                : co_await db.query("SELECT m.id, m.name, m.type, m.parent_id FROM sys_menu m "
                                    "INNER JOIN sys_role_menu rm ON m.id = rm.menu_id "
                                    "WHERE rm.role_id = ? AND m.deleted_at IS NULL "
                                    "ORDER BY m.`order` ASC, m.id ASC",
                                    {ruvia::DbValue{id}});
        for (const auto& mrow : mrs.rows()) {
            const auto mid = std::stoll(std::string(mrow[0].text()));
            menuIds.emplace_back(static_cast<ruvia::Int64>(mid));

            RoleMenuDto menu(c);
            menu.id(static_cast<ruvia::Int64>(mid));
            menu.name().assignView(mrow[1].text());
            menu.type().assignView(mrow[2].text());
            if (!mrow[3].isNull()) {
                menu.parentId(static_cast<ruvia::Int64>(std::stoll(std::string(mrow[3].text()))));
            }
            menus.push_back(std::move(menu));
        }
        out.menuIds(std::move(menuIds)).menus(std::move(menus));
        co_return out;
    }

    ruvia::Task<ruvia::List<RoleOptionDto>> listAllEnabled(ruvia::Context& c) {
        auto db = c.db();
        const auto rs =
            co_await db.query("SELECT id, name, code FROM sys_role "
                              "WHERE status = 'enabled' AND deleted_at IS NULL ORDER BY id ASC");
        ruvia::List<RoleOptionDto> out(c.resource());
        for (const auto& row : rs.rows()) {
            auto& item = out.emplace(c);
            item.id(static_cast<ruvia::Int64>(std::stoll(std::string(row[0].text()))));
            item.name().assignView(row[1].text());
            item.code().assignView(row[2].text());
        }
        co_return out;
    }

    ruvia::Task<void> create(ruvia::Context& c, const CreateRoleBody& body) {
        const std::string code(body.code()->view());
        const std::string name(body.name()->view());
        auto db = c.db();
        const auto exists = co_await db.query(
            "SELECT id FROM sys_role WHERE code = ? AND deleted_at IS NULL LIMIT 1",
            {ruvia::DbValue{code}});
        if (!exists.rows().empty())
            service::common::throwAppError(RoleError::CODE_EXISTS);

        const std::string status = body.status() ? std::string(body.status()->view()) : "enabled";
        const auto rs =
            co_await db.execute("INSERT INTO sys_role (name, code, status, created_at, updated_at) "
                                "VALUES (?, ?, ?, NOW(), NOW())",
                                {ruvia::DbValue{name}, ruvia::DbValue{code}, ruvia::DbValue{status}});
        const std::int64_t roleId = static_cast<std::int64_t>(rs.lastInsertId());

        if (code != service::common::kSuperAdminRoleCode) {
            co_await syncRoleMenus(c, roleId, body.menuIds());
        }
        service::middleware::permissionService().clearAllCache();
        co_return;
    }

    ruvia::Task<void> update(ruvia::Context& c, std::int64_t id, const UpdateRoleBody& body) {
        auto db = c.db();
        const auto rs = co_await db.query(
            "SELECT code FROM sys_role WHERE id = ? AND deleted_at IS NULL LIMIT 1",
            {ruvia::DbValue{id}});
        if (rs.rows().empty())
            service::common::throwAppError(RoleError::NOT_FOUND);
        const std::string currentCode(rs.rows().front()[0].text());

        const auto code = body.code() ? std::optional<std::string>(std::string(body.code()->view()))
                                      : std::nullopt;
        if (currentCode == service::common::kSuperAdminRoleCode && code &&
            *code != service::common::kSuperAdminRoleCode) {
            service::common::throwAppError(RoleError::SUPERADMIN_CANNOT_MODIFY);
        }

        std::string set;
        std::vector<ruvia::DbValue> params;
        auto append = [&](std::string_view col, ruvia::DbValue value) {
            if (!set.empty())
                set += ", ";
            set.append(col);
            set += " = ?";
            params.emplace_back(std::move(value));
        };
        if (body.name())
            append("name", ruvia::DbValue{std::string(body.name()->view())});
        if (code && *code != currentCode) {
            const auto existsRs = co_await db.query(
                "SELECT id FROM sys_role WHERE code = ? AND id != ? AND deleted_at IS NULL LIMIT 1",
                {ruvia::DbValue{*code}, ruvia::DbValue{id}});
            if (!existsRs.rows().empty())
                service::common::throwAppError(RoleError::CODE_EXISTS);
            append("code", ruvia::DbValue{*code});
        }
        if (body.status())
            append("status", ruvia::DbValue{std::string(body.status()->view())});

        if (!set.empty()) {
            params.emplace_back(ruvia::DbValue{id});
            (void)co_await db.execute(
                "UPDATE sys_role SET " + set + ", updated_at = NOW() WHERE id = ?", params);
        }

        const std::string effectiveCode = code.value_or(currentCode);
        if (body.menuIds()) {
            if (effectiveCode == service::common::kSuperAdminRoleCode) {
                (void)co_await db.execute("DELETE FROM sys_role_menu WHERE role_id = ?",
                                          {ruvia::DbValue{id}});
            } else {
                co_await syncRoleMenus(c, id, body.menuIds());
            }
        }
        service::middleware::permissionService().clearAllCache();
        co_return;
    }

    ruvia::Task<void> remove(ruvia::Context& c, std::int64_t id) {
        auto db = c.db();
        const auto rs = co_await db.query(
            "SELECT code FROM sys_role WHERE id = ? AND deleted_at IS NULL LIMIT 1",
            {ruvia::DbValue{id}});
        if (rs.rows().empty())
            service::common::throwAppError(RoleError::NOT_FOUND);
        const std::string code(rs.rows().front()[0].text());
        if (code == service::common::kSuperAdminRoleCode) {
            service::common::throwAppError(RoleError::SUPERADMIN_CANNOT_DELETE);
        }

        const auto userRs = co_await db.query(
            "SELECT COUNT(*) FROM sys_user_role WHERE role_id = ?", {ruvia::DbValue{id}});
        const std::int64_t userCount = std::stoll(std::string(userRs.rows().front()[0].text()));
        if (userCount > 0)
            service::common::throwAppError(RoleError::HAS_USERS);

        (void)co_await db.execute("UPDATE sys_role SET deleted_at = NOW() WHERE id = ?",
                                  {ruvia::DbValue{id}});
        service::middleware::permissionService().clearAllCache();
        co_return;
    }

    ruvia::Task<std::vector<std::int64_t>> getRoleMenuIds(ruvia::Context& c, std::int64_t roleId) {
        auto db = c.db();
        const auto rs = co_await db.query("SELECT menu_id FROM sys_role_menu WHERE role_id = ?",
                                          {ruvia::DbValue{roleId}});
        std::vector<std::int64_t> ids;
        ids.reserve(rs.rows().size());
        for (const auto& row : rs.rows()) {
            ids.push_back(std::stoll(std::string(row[0].text())));
        }
        co_return ids;
    }

  private:
    RoleService() = default;

    ruvia::Task<std::vector<std::int64_t>> getAllMenuIds(ruvia::Context& c) {
        auto db = c.db();
        const auto rs = co_await db.query(
            "SELECT id FROM sys_menu WHERE deleted_at IS NULL ORDER BY `order` ASC, id ASC");
        std::vector<std::int64_t> ids;
        ids.reserve(rs.rows().size());
        for (const auto& row : rs.rows()) {
            ids.push_back(std::stoll(std::string(row[0].text())));
        }
        co_return ids;
    }

    static void setMenuIds(ruvia::Context& c, RoleItemDto& item,
                           const std::vector<std::int64_t>& values) {
        ruvia::Array<ruvia::Int64> menuIds(c.allocator<ruvia::Int64>());
        menuIds.reserve(values.size());
        for (const auto value : values) {
            menuIds.emplace_back(static_cast<ruvia::Int64>(value));
        }
        item.menuIds(std::move(menuIds));
    }

    ruvia::Task<void> syncRoleMenus(ruvia::Context& c, std::int64_t roleId,
                                   const std::optional<ruvia::Array<ruvia::Int64>>& menuIds) {
        auto db = c.db();
        (void)co_await db.execute("DELETE FROM sys_role_menu WHERE role_id = ?",
                                  {ruvia::DbValue{roleId}});
        if (!menuIds)
            co_return;
        for (const auto menuId : *menuIds) {
            (void)co_await db.execute(
                "INSERT IGNORE INTO sys_role_menu (role_id, menu_id) VALUES (?, ?)",
                {ruvia::DbValue{roleId}, ruvia::DbValue{static_cast<std::int64_t>(menuId)}});
        }
        co_return;
    }
};

inline RoleService& roleService() { return RoleService::instance(); }

} // namespace service::role
