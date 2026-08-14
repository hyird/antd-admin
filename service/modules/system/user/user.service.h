#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <ruvia/core/Task.h>
#include <ruvia/web/db/Db.h>
#include <ruvia/web/Context.h>

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

    ruvia::Task<UserPageDataDto> list(ruvia::Context& c, std::int64_t page, std::int64_t pageSize,
                                      std::int64_t skip, const std::optional<std::string>& keyword,
                                      bool paginated, std::optional<std::string_view> status,
                                      std::optional<std::int64_t> deptId) {
        auto db = c.db();

        std::string where = " WHERE u.deleted_at IS NULL";
        std::vector<ruvia::DbValue> params;
        if (keyword) {
            where +=
                " AND (u.username LIKE ? OR u.nickname LIKE ? OR u.phone LIKE ? OR u.email LIKE ?)";
            const std::string like = "%" + service::common::escapeLikePattern(*keyword) + "%";
            for (int i = 0; i < 4; ++i)
                params.emplace_back(like);
        }
        if (status && !status->empty()) {
            where += " AND u.status = ?";
            params.emplace_back(*status);
        }
        if (deptId) {
            where += " AND u.dept_id = ?";
            params.emplace_back(*deptId);
        }

        const auto countRs = co_await db.query("SELECT COUNT(*) FROM sys_user u" + where, params);
        const std::int64_t total = std::stoll(std::string(countRs.front()[0].value().value_or("")));

        std::string sql = "SELECT u.id, u.username, u.nickname, u.phone, u.email, u.dept_id, "
                          "       u.status, d.name AS dept_name "
                          "FROM sys_user u LEFT JOIN sys_dept d ON u.dept_id = d.id" +
                          where + " ORDER BY u.id ASC";
        if (paginated) {
            sql += " LIMIT " + std::to_string(pageSize) + " OFFSET " + std::to_string(skip);
        }
        const auto rs = co_await db.query(sql, params);

        UserPageDataDto result(c);
        result.set<"total">(static_cast<ruvia::Int64>(total))
            .set<"page">(static_cast<ruvia::Int64>(page))
            .set<"pageSize">(static_cast<ruvia::Int64>(pageSize))
            .set<"totalPages">(static_cast<ruvia::Int64>(
                paginated && pageSize > 0 ? (total + pageSize - 1) / pageSize : 1));

        auto& list = result.ensure<"list">();
        for (const auto& row : rs) {
            auto& item = list.emplace_back(c);
            const auto userId = fillUserItem(item, row);
            co_await attachRoles(c, item, userId);
        }
        co_return result;
    }

    ruvia::Task<ruvia::Array<UserOptionDto>> listOptions(ruvia::Context& c,
                                                         std::optional<std::string_view> keyword) {
        auto db = c.db();
        std::string sql =
            "SELECT id, username, nickname, phone, email FROM sys_user WHERE deleted_at IS NULL";
        std::vector<ruvia::DbValue> params;
        if (keyword && !keyword->empty()) {
            sql += " AND (username LIKE ? OR nickname LIKE ? OR phone LIKE ? OR email LIKE ?)";
            const std::string like = "%" + service::common::escapeLikePattern(*keyword) + "%";
            for (int i = 0; i < 4; ++i)
                params.emplace_back(like);
        }
        sql += " ORDER BY id ASC";
        const auto rs = co_await db.query(sql, params);
        ruvia::Array<UserOptionDto> out(c.allocator<UserOptionDto>());
        for (const auto& row : rs) {
            auto& item = out.emplace_back(c);
            item.set<"id">(
                static_cast<ruvia::Int64>(std::stoll(std::string(row[0].value().value_or("")))));
            item.set<"username">(row[1].value().value_or(""));
            if (row[2].value().has_value())
                item.set<"nickname">(row[2].value().value_or(""));
            if (row[3].value().has_value())
                item.set<"phone">(row[3].value().value_or(""));
            if (row[4].value().has_value())
                item.set<"email">(row[4].value().value_or(""));
        }
        co_return out;
    }

    ruvia::Task<UserItemDto> getById(ruvia::Context& c, std::int64_t id) {
        auto db = c.db();
        const auto rs = co_await db.query(
            "SELECT u.id, u.username, u.nickname, u.phone, u.email, u.dept_id, u.status, "
            "       d.name "
            "FROM sys_user u LEFT JOIN sys_dept d ON u.dept_id = d.id "
            "WHERE u.id = ? AND u.deleted_at IS NULL LIMIT 1",
            service::common::dbParams(ruvia::DbValue{id}));
        if (rs.empty())
            service::common::throwAppError(UserError::USER_NOT_FOUND);

        UserItemDto item(c);
        const auto userId = fillUserItem(item, rs.front());
        co_await attachRoles(c, item, userId);
        co_return item;
    }

    ruvia::Task<void> create(ruvia::Context& c, const CreateUserBody& body) {
        auto db = c.db();
        const std::string username(body.get<"username">()->view());
        const std::string phone =
            body.get<"phone">() ? std::string(body.get<"phone">()->view()) : std::string{};
        const std::string email =
            body.get<"email">() ? std::string(body.get<"email">()->view()) : std::string{};
        const auto existing = co_await db.query(
            "SELECT id FROM sys_user WHERE username = ? AND deleted_at IS NULL LIMIT 1",
            service::common::dbParams(ruvia::DbValue{username}));
        if (!existing.empty())
            service::common::throwAppError(UserError::USERNAME_EXISTS);

        if (!phone.empty())
            co_await checkPhoneUnique(c, phone, 0);
        if (!email.empty())
            co_await checkEmailUnique(c, email, 0);

        if (!body.get<"roleIds">() || body.get<"roleIds">()->empty())
            service::common::throwAppError(UserError::ROLE_REQUIRED);

        const auto hash = service::utils::hashPassword(body.get<"password">()->view());
        auto tx = co_await db.beginTransaction();
        const auto rs = co_await tx.execute(
            "INSERT INTO sys_user (username, password_hash, nickname, phone, email, "
            "                     dept_id, status, created_at, updated_at) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, NOW(), NOW())",
            service::common::dbParams(
                ruvia::DbValue{username}, ruvia::DbValue{hash},
                body.get<"nickname">() ? ruvia::DbValue{body.get<"nickname">()->view()}
                                       : ruvia::DbValue{nullptr},
                !phone.empty() ? ruvia::DbValue{phone} : ruvia::DbValue{nullptr},
                !email.empty() ? ruvia::DbValue{email} : ruvia::DbValue{nullptr},
                body.get<"deptId">()
                    ? ruvia::DbValue{static_cast<std::int64_t>(*body.get<"deptId">())}
                    : ruvia::DbValue{nullptr},
                ruvia::DbValue{body.get<"status">() ? body.get<"status">()->view()
                                                    : std::string_view{"enabled"}}));
        const std::int64_t userId = static_cast<std::int64_t>(rs.lastInsertId().value_or(0));

        for (const auto roleId : *body.get<"roleIds">()) {
            (void)co_await tx.execute(
                "INSERT IGNORE INTO sys_user_role (user_id, role_id) VALUES (?, ?)",
                service::common::dbParams(ruvia::DbValue{userId},
                                          ruvia::DbValue{static_cast<std::int64_t>(roleId)}));
        }
        co_await tx.commit();
        co_return;
    }

    ruvia::Task<void> update(ruvia::Context& c, std::int64_t id, const UpdateUserBody& body) {
        auto db = c.db();
        const auto rs = co_await db.query(
            "SELECT username FROM sys_user WHERE id = ? AND deleted_at IS NULL LIMIT 1",
            service::common::dbParams(ruvia::DbValue{id}));
        if (rs.empty())
            service::common::throwAppError(UserError::USER_NOT_FOUND);
        const std::string username(rs.front()[0].value().value_or(""));
        if (username == "admin" && body.get<"roleIds">()) {
            service::common::throwAppError(UserError::ADMIN_ROLE_PROTECTED);
        }

        const std::string phone =
            body.get<"phone">() ? std::string(body.get<"phone">()->view()) : std::string{};
        const std::string email =
            body.get<"email">() ? std::string(body.get<"email">()->view()) : std::string{};
        if (!phone.empty())
            co_await checkPhoneUnique(c, phone, id);
        if (!email.empty())
            co_await checkEmailUnique(c, email, id);

        if (body.get<"roleIds">() && body.get<"roleIds">()->empty())
            service::common::throwAppError(UserError::ROLE_REQUIRED);

        std::string set;
        std::vector<ruvia::DbValue> params;
        // Kept at function scope because DbValue borrows its string; the hash must
        // outlive the UPDATE below.
        std::string passwordHash;
        auto append = [&](std::string_view col, ruvia::DbValue value) {
            if (!set.empty())
                set += ", ";
            set.append(col);
            set += " = ?";
            params.emplace_back(std::move(value));
        };
        if (body.get<"nickname">())
            append("nickname", ruvia::DbValue{body.get<"nickname">()->view()});
        if (body.get<"phone">())
            append("phone", ruvia::DbValue{phone});
        if (body.get<"email">())
            append("email", ruvia::DbValue{email});
        if (body.get<"deptId">())
            append("dept_id", ruvia::DbValue{static_cast<std::int64_t>(*body.get<"deptId">())});
        if (body.get<"status">())
            append("status", ruvia::DbValue{body.get<"status">()->view()});
        if (body.get<"password">() && !body.get<"password">()->empty()) {
            passwordHash = service::utils::hashPassword(body.get<"password">()->view());
            append("password_hash", ruvia::DbValue{passwordHash});
        }
        auto tx = co_await db.beginTransaction();
        if (!set.empty()) {
            params.emplace_back(ruvia::DbValue{id});
            (void)co_await tx.execute(
                "UPDATE sys_user SET " + set + ", updated_at = NOW() WHERE id = ?", params);
        }

        if (body.get<"roleIds">()) {
            (void)co_await tx.execute("DELETE FROM sys_user_role WHERE user_id = ?",
                                      service::common::dbParams(ruvia::DbValue{id}));
            for (const auto roleId : *body.get<"roleIds">()) {
                (void)co_await tx.execute(
                    "INSERT IGNORE INTO sys_user_role (user_id, role_id) VALUES (?, ?)",
                    service::common::dbParams(ruvia::DbValue{id},
                                              ruvia::DbValue{static_cast<std::int64_t>(roleId)}));
            }
        }
        co_await tx.commit();
        if (body.get<"roleIds">())
            service::middleware::permissionService().clearUserCache(id);
        co_return;
    }

    ruvia::Task<void> remove(ruvia::Context& c, std::int64_t id) {
        auto db = c.db();
        const auto rs = co_await db.query(
            "SELECT username FROM sys_user WHERE id = ? AND deleted_at IS NULL LIMIT 1",
            service::common::dbParams(ruvia::DbValue{id}));
        if (rs.empty())
            service::common::throwAppError(UserError::USER_NOT_FOUND);
        if (std::string(rs.front()[0].value().value_or("")) == "admin") {
            service::common::throwAppError(UserError::ADMIN_DELETE_PROTECTED);
        }
        (void)co_await db.execute("UPDATE sys_user SET deleted_at = NOW() WHERE id = ?",
                                  service::common::dbParams(ruvia::DbValue{id}));
        service::middleware::permissionService().clearUserCache(id);
        co_return;
    }

  private:
    UserService() = default;

    template <typename Row> static std::int64_t fillUserItem(UserItemDto& item, const Row& row) {
        const std::int64_t userId = std::stoll(std::string(row[0].value().value_or("")));
        item.set<"id">(static_cast<ruvia::Int64>(userId));
        item.set<"username">(row[1].value().value_or(""));
        item.set<"status">(row[6].value().value_or(""));
        if (row[2].value().has_value())
            item.set<"nickname">(row[2].value().value_or(""));
        if (row[3].value().has_value())
            item.set<"phone">(row[3].value().value_or(""));
        if (row[4].value().has_value())
            item.set<"email">(row[4].value().value_or(""));
        if (row[5].value().has_value()) {
            item.set<"deptId">(
                static_cast<ruvia::Int64>(std::stoll(std::string(row[5].value().value_or("")))));
        }
        if (row[7].value().has_value())
            item.set<"deptName">(row[7].value().value_or(""));
        return userId;
    }

    ruvia::Task<void> attachRoles(ruvia::Context& c, UserItemDto& item, std::int64_t userId) {
        auto db = c.db();
        const auto roles = co_await db.query("SELECT r.id, r.name, r.code FROM sys_role r "
                                             "INNER JOIN sys_user_role ur ON r.id = ur.role_id "
                                             "WHERE ur.user_id = ? AND r.deleted_at IS NULL",
                                             service::common::dbParams(ruvia::DbValue{userId}));
        auto& roleList = item.ensure<"roles">();
        for (const auto& rrow : roles) {
            auto& role = roleList.emplace_back(c);
            role.set<"id">(
                static_cast<ruvia::Int64>(std::stoll(std::string(rrow[0].value().value_or("")))));
            role.set<"name">(rrow[1].value().value_or(""));
            role.set<"code">(rrow[2].value().value_or(""));
        }
        co_return;
    }

    ruvia::Task<void> checkPhoneUnique(ruvia::Context& c, const std::string& phone,
                                       std::int64_t excludeId) {
        if (phone.empty())
            co_return;
        auto db = c.db();
        const auto rs = co_await db.query(
            "SELECT id FROM sys_user WHERE phone = ? AND deleted_at IS NULL AND id != ? LIMIT 1",
            service::common::dbParams(ruvia::DbValue{phone}, ruvia::DbValue{excludeId}));
        if (!rs.empty())
            service::common::throwAppError(UserError::PHONE_EXISTS);
        co_return;
    }

    ruvia::Task<void> checkEmailUnique(ruvia::Context& c, const std::string& email,
                                       std::int64_t excludeId) {
        if (email.empty())
            co_return;
        auto db = c.db();
        const auto rs = co_await db.query(
            "SELECT id FROM sys_user WHERE email = ? AND deleted_at IS NULL AND id != ? LIMIT 1",
            service::common::dbParams(ruvia::DbValue{email}, ruvia::DbValue{excludeId}));
        if (!rs.empty())
            service::common::throwAppError(UserError::EMAIL_EXISTS);
        co_return;
    }
};

inline UserService& userService() { return UserService::instance(); }

} // namespace service::user
