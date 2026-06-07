#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <cyra/app/Task.h>
#include <cyra/db/Db.h>
#include <cyra/http/Context.h>

#include "service/common/http.h"
#include "service/common/types.h"
#include "service/middleware/permission.h"
#include "service/modules/system/role/role.error.h"
#include "service/modules/system/role/role.types.h"

namespace service::modules::system::role {

class RoleService {
public:
    static RoleService& instance() {
        static RoleService svc;
        return svc;
    }

    cyra::Task<RolePageDataDto> list(cyra::Context& c, const RoleQuery& q) {
        const auto p = service::common::normalizePagination(q);
        auto db = c.db();

        std::string where = " FROM sys_role r WHERE r.deleted_at IS NULL";
        std::vector<cyra::DbValue> params;
        if (p.keyword) {
            where += " AND (r.name LIKE ? OR r.code LIKE ?)";
            const std::string like = "%" + *p.keyword + "%";
            params.emplace_back(like);
            params.emplace_back(like);
        }
        if (q.status) {
            where += " AND r.status = ?";
            params.emplace_back(*q.status);
        }

        const auto countRs = co_await db.query("SELECT COUNT(*)" + where, params);
        const std::int64_t total = countRs.rows().empty()
                                       ? 0
                                       : std::stoll(std::string(countRs.rows().front()[0].text()));

        std::string sql = "SELECT r.id, r.name, r.code, r.status" + where + " ORDER BY r.id ASC";
        if (p.paginated) {
            sql += " LIMIT " + std::to_string(p.page_size) + " OFFSET " + std::to_string(p.skip);
        }
        const auto rs = co_await db.query(sql, params);

        RolePageDataDto result(c);
        result.total(static_cast<cyra::Int64>(total))
            .page(static_cast<cyra::Int64>(p.page))
            .pageSize(static_cast<cyra::Int64>(p.page_size))
            .totalPages(static_cast<cyra::Int64>(
                p.paginated && p.page_size > 0 ? (total + p.page_size - 1) / p.page_size : 1));

        auto& list = result.listEnsure();
        for (const auto& row : rs.rows()) {
            auto& item = list.emplace(c);
            const auto id = std::stoll(std::string(row[0].text()));
            item.id(static_cast<cyra::Int64>(id))
                .name(row[1].text())
                .code(row[2].text())
                .status(row[3].text());
            setMenuIds(c, item, co_await getRoleMenuIds(c, id));
        }
        co_return result;
    }

    cyra::Task<RoleDetailDto> getById(cyra::Context& c, std::int64_t id) {
        auto db = c.db();
        const auto rs = co_await db.query(
            "SELECT id, name, code, status FROM sys_role WHERE id = ? AND deleted_at IS NULL LIMIT 1",
            {cyra::DbValue{id}});
        if (rs.rows().empty()) service::common::throwAppError(RoleError::NOT_FOUND);

        const auto& row = rs.rows().front();
        RoleDetailDto out(c);
        out.id(static_cast<cyra::Int64>(std::stoll(std::string(row[0].text()))))
            .name(row[1].text())
            .code(row[2].text())
            .status(row[3].text());

        cyra::Array<cyra::Int64> menuIds(c.allocator<cyra::Int64>());
        cyra::Array<RoleMenuDto> menus(c.allocator<RoleMenuDto>());
        const auto mrs = co_await db.query(
            "SELECT m.id, m.name, m.type, m.parent_id FROM sys_menu m "
            "INNER JOIN sys_role_menu rm ON m.id = rm.menu_id "
            "WHERE rm.role_id = ? AND m.deleted_at IS NULL",
            {cyra::DbValue{id}});
        for (const auto& mrow : mrs.rows()) {
            const auto mid = std::stoll(std::string(mrow[0].text()));
            menuIds.emplace_back(static_cast<cyra::Int64>(mid));

            RoleMenuDto menu(c);
            menu.id(static_cast<cyra::Int64>(mid))
                .name(mrow[1].text())
                .type(mrow[2].text());
            if (!mrow[3].isNull()) {
                menu.parentId(static_cast<cyra::Int64>(std::stoll(std::string(mrow[3].text()))));
            }
            menus.push_back(std::move(menu));
        }
        out.menuIds(std::move(menuIds)).menus(std::move(menus));
        co_return out;
    }

    cyra::Task<cyra::List<RoleOptionDto>> listAllEnabled(cyra::Context& c) {
        auto db = c.db();
        const auto rs = co_await db.query(
            "SELECT id, name, code FROM sys_role "
            "WHERE status = 'enabled' AND deleted_at IS NULL ORDER BY id ASC");
        cyra::List<RoleOptionDto> out(c.resource());
        for (const auto& row : rs.rows()) {
            auto& item = out.emplace(c);
            item.id(static_cast<cyra::Int64>(std::stoll(std::string(row[0].text()))))
                .name(row[1].text())
                .code(row[2].text());
        }
        co_return out;
    }

    cyra::Task<void> create(cyra::Context& c, const CreateRoleBody& body) {
        const std::string code(body.code()->view());
        const std::string name(body.name()->view());
        auto db = c.db();
        const auto exists = co_await db.query(
            "SELECT id FROM sys_role WHERE code = ? AND deleted_at IS NULL LIMIT 1",
            {cyra::DbValue{code}});
        if (!exists.rows().empty()) service::common::throwAppError(RoleError::CODE_EXISTS);

        const std::string status = body.status() ? std::string(body.status()->view()) : "enabled";
        const auto rs = co_await db.execute(
            "INSERT INTO sys_role (name, code, status, created_at, updated_at) "
            "VALUES (?, ?, ?, NOW(), NOW())",
            {cyra::DbValue{name}, cyra::DbValue{code}, cyra::DbValue{status}});
        const std::int64_t roleId = static_cast<std::int64_t>(rs.lastInsertId());

        co_await syncRoleMenus(c, roleId, body.menuIds());
        service::middleware::permissionService().clearAllCache();
        co_return;
    }

    cyra::Task<void> update(cyra::Context& c, std::int64_t id, const UpdateRoleBody& body) {
        auto db = c.db();
        const auto rs = co_await db.query(
            "SELECT code FROM sys_role WHERE id = ? AND deleted_at IS NULL LIMIT 1",
            {cyra::DbValue{id}});
        if (rs.rows().empty()) service::common::throwAppError(RoleError::NOT_FOUND);
        const std::string currentCode(rs.rows().front()[0].text());

        const auto code = body.code() ? std::optional<std::string>(std::string(body.code()->view()))
                                      : std::nullopt;
        if (currentCode == service::common::kSuperAdminRoleCode && code &&
            *code != service::common::kSuperAdminRoleCode) {
            service::common::throwAppError(RoleError::SUPERADMIN_CANNOT_MODIFY);
        }

        std::string set;
        std::vector<cyra::DbValue> params;
        auto append = [&](std::string_view col, cyra::DbValue value) {
            if (!set.empty()) set += ", ";
            set.append(col);
            set += " = ?";
            params.emplace_back(std::move(value));
        };
        if (body.name()) append("name", cyra::DbValue{std::string(body.name()->view())});
        if (code && *code != currentCode) {
            const auto existsRs = co_await db.query(
                "SELECT id FROM sys_role WHERE code = ? AND id != ? AND deleted_at IS NULL LIMIT 1",
                {cyra::DbValue{*code}, cyra::DbValue{id}});
            if (!existsRs.rows().empty()) service::common::throwAppError(RoleError::CODE_EXISTS);
            append("code", cyra::DbValue{*code});
        }
        if (body.status()) append("status", cyra::DbValue{std::string(body.status()->view())});

        if (!set.empty()) {
            params.emplace_back(cyra::DbValue{id});
            (void)co_await db.execute("UPDATE sys_role SET " + set + ", updated_at = NOW() WHERE id = ?",
                                    params);
        }

        if (body.menuIds()) {
            if (currentCode == service::common::kSuperAdminRoleCode) {
                (void)co_await db.execute("DELETE FROM sys_role_menu WHERE role_id = ?",
                                          {cyra::DbValue{id}});
            } else {
                co_await syncRoleMenus(c, id, body.menuIds());
            }
        }
        service::middleware::permissionService().clearAllCache();
        co_return;
    }

    cyra::Task<void> remove(cyra::Context& c, std::int64_t id) {
        auto db = c.db();
        const auto rs = co_await db.query(
            "SELECT code FROM sys_role WHERE id = ? AND deleted_at IS NULL LIMIT 1",
            {cyra::DbValue{id}});
        if (rs.rows().empty()) service::common::throwAppError(RoleError::NOT_FOUND);
        const std::string code(rs.rows().front()[0].text());
        if (code == service::common::kSuperAdminRoleCode) {
            service::common::throwAppError(RoleError::SUPERADMIN_CANNOT_DELETE);
        }

        const auto userRs = co_await db.query(
            "SELECT COUNT(*) FROM sys_user_role WHERE role_id = ?", {cyra::DbValue{id}});
        const std::int64_t userCount = std::stoll(std::string(userRs.rows().front()[0].text()));
        if (userCount > 0) service::common::throwAppError(RoleError::HAS_USERS);

        (void)co_await db.execute("UPDATE sys_role SET deleted_at = NOW() WHERE id = ?",
                                {cyra::DbValue{id}});
        service::middleware::permissionService().clearAllCache();
        co_return;
    }

    cyra::Task<std::vector<std::int64_t>> getRoleMenuIds(cyra::Context& c, std::int64_t roleId) {
        auto db = c.db();
        const auto rs = co_await db.query(
            "SELECT menu_id FROM sys_role_menu WHERE role_id = ?", {cyra::DbValue{roleId}});
        std::vector<std::int64_t> ids;
        ids.reserve(rs.rows().size());
        for (const auto& row : rs.rows()) {
            ids.push_back(std::stoll(std::string(row[0].text())));
        }
        co_return ids;
    }

private:
    RoleService() = default;

    static void setMenuIds(cyra::Context& c,
                           RoleItemDto& item,
                           const std::vector<std::int64_t>& values) {
        cyra::Array<cyra::Int64> menuIds(c.allocator<cyra::Int64>());
        menuIds.reserve(values.size());
        for (const auto value : values) {
            menuIds.emplace_back(static_cast<cyra::Int64>(value));
        }
        item.menuIds(std::move(menuIds));
    }

    cyra::Task<void> syncRoleMenus(cyra::Context& c,
                                   std::int64_t roleId,
                                   const std::optional<cyra::Array<cyra::Int64>>& menuIds) {
        auto db = c.db();
        (void)co_await db.execute("DELETE FROM sys_role_menu WHERE role_id = ?", {cyra::DbValue{roleId}});
        if (!menuIds) co_return;
        for (const auto menuId : *menuIds) {
            (void)co_await db.execute("INSERT IGNORE INTO sys_role_menu (role_id, menu_id) VALUES (?, ?)",
                                    {cyra::DbValue{roleId},
                                     cyra::DbValue{static_cast<std::int64_t>(menuId)}});
        }
        co_return;
    }
};

inline RoleService& roleService() { return RoleService::instance(); }

}  // namespace service::modules::system::role
