#pragma once

#include <cstdint>
#include <optional>
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
#include "service/modules/system/user/user.error.h"
#include "service/modules/system/user/user.types.h"
#include "service/utils/password.h"

namespace service::user {

class UserService {
  public:
    static UserService& instance() {
        static UserService svc;
        return svc;
    }

    cyra::Task<UserPageDataDto> list(cyra::Context& c, std::int64_t page, std::int64_t pageSize,
                                     std::int64_t skip, const std::optional<std::string>& keyword,
                                     bool paginated, std::optional<std::string_view> status,
                                     std::optional<std::int64_t> deptId) {
        auto db = c.db();

        std::string where = " WHERE u.deleted_at IS NULL";
        std::vector<cyra::DbValue> params;
        if (keyword) {
            where +=
                " AND (u.username LIKE ? OR u.nickname LIKE ? OR u.phone LIKE ? OR u.email LIKE ?)";
            const std::string like = "%" + *keyword + "%";
            for (int i = 0; i < 4; ++i)
                params.emplace_back(like);
        }
        if (status && !status->empty()) {
            where += " AND u.status = ?";
            params.emplace_back(std::string(*status));
        }
        if (deptId) {
            where += " AND u.dept_id = ?";
            params.emplace_back(*deptId);
        }

        const auto countRs = co_await db.query("SELECT COUNT(*) FROM sys_user u" + where, params);
        const std::int64_t total = std::stoll(std::string(countRs.rows().front()[0].text()));

        std::string sql = "SELECT u.id, u.username, u.nickname, u.phone, u.email, u.dept_id, "
                          "       u.status, d.name AS dept_name "
                          "FROM sys_user u LEFT JOIN sys_dept d ON u.dept_id = d.id" +
                          where + " ORDER BY u.id ASC";
        if (paginated) {
            sql += " LIMIT " + std::to_string(pageSize) + " OFFSET " + std::to_string(skip);
        }
        const auto rs = co_await db.query(sql, params);

        UserPageDataDto result(c);
        result.total(static_cast<cyra::Int64>(total))
            .page(static_cast<cyra::Int64>(page))
            .pageSize(static_cast<cyra::Int64>(pageSize))
            .totalPages(static_cast<cyra::Int64>(
                paginated && pageSize > 0 ? (total + pageSize - 1) / pageSize : 1));

        auto& list = result.list().ensure();
        for (const auto& row : rs.rows()) {
            auto& item = list.emplace(c);
            const auto userId = fillUserItem(item, row);
            co_await attachRoles(c, item, userId);
        }
        co_return result;
    }

    cyra::Task<cyra::List<UserOptionDto>> listOptions(cyra::Context& c,
                                                      std::optional<std::string_view> keyword) {
        auto db = c.db();
        std::string sql =
            "SELECT id, username, nickname, phone, email FROM sys_user WHERE deleted_at IS NULL";
        std::vector<cyra::DbValue> params;
        if (keyword && !keyword->empty()) {
            sql += " AND (username LIKE ? OR nickname LIKE ? OR phone LIKE ? OR email LIKE ?)";
            const std::string like = "%" + std::string(*keyword) + "%";
            for (int i = 0; i < 4; ++i)
                params.emplace_back(like);
        }
        sql += " ORDER BY id ASC";
        const auto rs = co_await db.query(sql, params);
        cyra::List<UserOptionDto> out(c.resource());
        for (const auto& row : rs.rows()) {
            auto& item = out.emplace(c);
            item.id(static_cast<cyra::Int64>(std::stoll(std::string(row[0].text()))));
            item.username().assignView(row[1].text());
            if (!row[2].isNull())
                item.nickname().assignView(row[2].text());
            if (!row[3].isNull())
                item.phone().assignView(row[3].text());
            if (!row[4].isNull())
                item.email().assignView(row[4].text());
        }
        co_return out;
    }

    cyra::Task<UserItemDto> getById(cyra::Context& c, std::int64_t id) {
        auto db = c.db();
        const auto rs = co_await db.query(
            "SELECT u.id, u.username, u.nickname, u.phone, u.email, u.dept_id, u.status, "
            "       d.name "
            "FROM sys_user u LEFT JOIN sys_dept d ON u.dept_id = d.id "
            "WHERE u.id = ? AND u.deleted_at IS NULL LIMIT 1",
            {cyra::DbValue{id}});
        if (rs.rows().empty())
            service::common::throwAppError(UserError::USER_NOT_FOUND);

        UserItemDto item(c);
        const auto userId = fillUserItem(item, rs.rows().front());
        co_await attachRoles(c, item, userId);
        co_return item;
    }

    cyra::Task<void> create(cyra::Context& c, const CreateUserBody& body) {
        auto db = c.db();
        const std::string username(body.username()->view());
        const std::string phone = body.phone() ? std::string(body.phone()->view()) : std::string{};
        const std::string email = body.email() ? std::string(body.email()->view()) : std::string{};
        const auto existing = co_await db.query(
            "SELECT id FROM sys_user WHERE username = ? AND deleted_at IS NULL LIMIT 1",
            {cyra::DbValue{username}});
        if (!existing.rows().empty())
            service::common::throwAppError(UserError::USERNAME_EXISTS);

        if (!phone.empty())
            co_await checkPhoneUnique(c, phone, 0);
        if (!email.empty())
            co_await checkEmailUnique(c, email, 0);

        if (!body.roleIds() || body.roleIds()->empty())
            service::common::throwAppError(UserError::ROLE_REQUIRED);

        const auto hash = service::utils::hashPassword(body.password()->view());
        const auto rs = co_await db.execute(
            "INSERT INTO sys_user (username, password_hash, nickname, phone, email, "
            "                     dept_id, status, created_at, updated_at) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, NOW(), NOW())",
            {cyra::DbValue{username}, cyra::DbValue{hash},
             body.nickname() ? cyra::DbValue{std::string(body.nickname()->view())}
                             : cyra::DbValue{nullptr},
             !phone.empty() ? cyra::DbValue{phone} : cyra::DbValue{nullptr},
             !email.empty() ? cyra::DbValue{email} : cyra::DbValue{nullptr},
             body.deptId() ? cyra::DbValue{static_cast<std::int64_t>(*body.deptId())}
                           : cyra::DbValue{nullptr},
             cyra::DbValue{body.status() ? std::string(body.status()->view()) : "enabled"}});
        const std::int64_t userId = static_cast<std::int64_t>(rs.lastInsertId());

        for (const auto roleId : *body.roleIds()) {
            (void)co_await db.execute(
                "INSERT IGNORE INTO sys_user_role (user_id, role_id) VALUES (?, ?)",
                {cyra::DbValue{userId}, cyra::DbValue{static_cast<std::int64_t>(roleId)}});
        }
        co_return;
    }

    cyra::Task<void> update(cyra::Context& c, std::int64_t id, const UpdateUserBody& body) {
        auto db = c.db();
        const auto rs = co_await db.query(
            "SELECT username FROM sys_user WHERE id = ? AND deleted_at IS NULL LIMIT 1",
            {cyra::DbValue{id}});
        if (rs.rows().empty())
            service::common::throwAppError(UserError::USER_NOT_FOUND);
        const std::string username(rs.rows().front()[0].text());
        if (username == "admin" && body.roleIds()) {
            service::common::throwAppError(UserError::ADMIN_ROLE_PROTECTED);
        }

        const std::string phone = body.phone() ? std::string(body.phone()->view()) : std::string{};
        const std::string email = body.email() ? std::string(body.email()->view()) : std::string{};
        if (!phone.empty())
            co_await checkPhoneUnique(c, phone, id);
        if (!email.empty())
            co_await checkEmailUnique(c, email, id);

        std::string set;
        std::vector<cyra::DbValue> params;
        auto append = [&](std::string_view col, cyra::DbValue value) {
            if (!set.empty())
                set += ", ";
            set.append(col);
            set += " = ?";
            params.emplace_back(std::move(value));
        };
        if (body.nickname())
            append("nickname", cyra::DbValue{std::string(body.nickname()->view())});
        if (body.phone())
            append("phone", cyra::DbValue{phone});
        if (body.email())
            append("email", cyra::DbValue{email});
        if (body.deptId())
            append("dept_id", cyra::DbValue{static_cast<std::int64_t>(*body.deptId())});
        if (body.status())
            append("status", cyra::DbValue{std::string(body.status()->view())});
        if (body.password() && !body.password()->empty()) {
            const auto hash = service::utils::hashPassword(body.password()->view());
            append("password_hash", cyra::DbValue{hash});
        }
        if (!set.empty()) {
            params.emplace_back(cyra::DbValue{id});
            (void)co_await db.execute(
                "UPDATE sys_user SET " + set + ", updated_at = NOW() WHERE id = ?", params);
        }

        if (body.roleIds()) {
            if (body.roleIds()->empty())
                service::common::throwAppError(UserError::ROLE_REQUIRED);
            (void)co_await db.execute("DELETE FROM sys_user_role WHERE user_id = ?",
                                      {cyra::DbValue{id}});
            for (const auto roleId : *body.roleIds()) {
                (void)co_await db.execute(
                    "INSERT IGNORE INTO sys_user_role (user_id, role_id) VALUES (?, ?)",
                    {cyra::DbValue{id}, cyra::DbValue{static_cast<std::int64_t>(roleId)}});
            }
            service::middleware::permissionService().clearUserCache(id);
        }
        co_return;
    }

    cyra::Task<void> remove(cyra::Context& c, std::int64_t id) {
        auto db = c.db();
        const auto rs = co_await db.query(
            "SELECT username FROM sys_user WHERE id = ? AND deleted_at IS NULL LIMIT 1",
            {cyra::DbValue{id}});
        if (rs.rows().empty())
            service::common::throwAppError(UserError::USER_NOT_FOUND);
        if (std::string(rs.rows().front()[0].text()) == "admin") {
            service::common::throwAppError(UserError::ADMIN_DELETE_PROTECTED);
        }
        (void)co_await db.execute("UPDATE sys_user SET deleted_at = NOW() WHERE id = ?",
                                  {cyra::DbValue{id}});
        service::middleware::permissionService().clearUserCache(id);
        co_return;
    }

  private:
    UserService() = default;

    template <typename Row> static std::int64_t fillUserItem(UserItemDto& item, const Row& row) {
        const std::int64_t userId = std::stoll(std::string(row[0].text()));
        item.id(static_cast<cyra::Int64>(userId));
        item.username().assignView(row[1].text());
        item.status().assignView(row[6].text());
        if (!row[2].isNull())
            item.nickname().assignView(row[2].text());
        if (!row[3].isNull())
            item.phone().assignView(row[3].text());
        if (!row[4].isNull())
            item.email().assignView(row[4].text());
        if (!row[5].isNull()) {
            item.deptId(static_cast<cyra::Int64>(std::stoll(std::string(row[5].text()))));
        }
        if (!row[7].isNull())
            item.deptName().assignView(row[7].text());
        return userId;
    }

    cyra::Task<void> attachRoles(cyra::Context& c, UserItemDto& item, std::int64_t userId) {
        auto db = c.db();
        const auto roles = co_await db.query("SELECT r.id, r.name, r.code FROM sys_role r "
                                             "INNER JOIN sys_user_role ur ON r.id = ur.role_id "
                                             "WHERE ur.user_id = ? AND r.deleted_at IS NULL",
                                             {cyra::DbValue{userId}});
        auto& roleList = item.roles().ensure();
        for (const auto& rrow : roles.rows()) {
            auto& role = roleList.emplace(c);
            role.id(static_cast<cyra::Int64>(std::stoll(std::string(rrow[0].text()))));
            role.name().assignView(rrow[1].text());
            role.code().assignView(rrow[2].text());
        }
        co_return;
    }

    cyra::Task<void> checkPhoneUnique(cyra::Context& c, const std::string& phone,
                                      std::int64_t excludeId) {
        if (phone.empty())
            co_return;
        auto db = c.db();
        const auto rs = co_await db.query(
            "SELECT id FROM sys_user WHERE phone = ? AND deleted_at IS NULL AND id != ? LIMIT 1",
            {cyra::DbValue{phone}, cyra::DbValue{excludeId}});
        if (!rs.rows().empty())
            service::common::throwAppError(UserError::PHONE_EXISTS);
        co_return;
    }

    cyra::Task<void> checkEmailUnique(cyra::Context& c, const std::string& email,
                                      std::int64_t excludeId) {
        if (email.empty())
            co_return;
        auto db = c.db();
        const auto rs = co_await db.query(
            "SELECT id FROM sys_user WHERE email = ? AND deleted_at IS NULL AND id != ? LIMIT 1",
            {cyra::DbValue{email}, cyra::DbValue{excludeId}});
        if (!rs.rows().empty())
            service::common::throwAppError(UserError::EMAIL_EXISTS);
        co_return;
    }
};

inline UserService& userService() { return UserService::instance(); }

} // namespace service::user
