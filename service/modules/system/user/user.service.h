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
#include "service/modules/system/user/user.error.h"
#include "service/modules/system/user/user.schema.h"
#include "service/modules/system/user/user.types.h"
#include "service/utils/password.h"

namespace service::user {

using ruvia::db_expr;

class UserService {
  public:
    static UserService& instance() {
        static UserService svc;
        return svc;
    }

    ruvia::task<UserPageDataDto> list(ruvia::context& c, std::int64_t page, std::int64_t pageSize,
                                      std::int64_t skip, const std::optional<std::string>& keyword,
                                      bool paginated, std::optional<std::string_view> status,
                                      std::optional<std::int64_t> deptId) {
        auto db = c.db();

        ruvia::db_query query;
        query.from(storage::User::table_name(), "u")
            .where(storage::User::column<"deleted_at">("u").is_null());
        if (keyword) {
            const auto like = "%" + ruvia::escape_like_pattern(*keyword) + "%";
            query.and_where(storage::User::column<"username">("u").like(like) ||
                            storage::User::column<"nickname">("u").like(like) ||
                            storage::User::column<"phone">("u").like(like) ||
                            storage::User::column<"email">("u").like(like));
        }
        if (status && !status->empty())
            query.and_where(storage::User::column<"status">("u").to_expr() == *status);
        if (deptId)
            query.and_where(storage::User::column<"dept_id">("u").to_expr() == *deptId);

        query.select(db_expr::count());
        const auto countRows = co_await db.query(query);
        const auto total = countRows.front()[0].as<std::int64_t>().value_or(0);

        query.join(ruvia::db_join_type::left, storage::Department::table_name(),
                   storage::User::column<"dept_id">("u").to_expr() ==
                       storage::Department::column<"id">("d").to_expr(),
                   "d");
        query.select({storage::User::column<"id">("u").to_expr(),
                      storage::User::column<"username">("u").to_expr(),
                      storage::User::column<"nickname">("u").to_expr(),
                      storage::User::column<"phone">("u").to_expr(),
                      storage::User::column<"email">("u").to_expr(),
                      storage::User::column<"dept_id">("u").to_expr(),
                      storage::User::column<"status">("u").to_expr(),
                      storage::Department::column<"name">("d").to_expr().as("dept_name")});
        query.order_by(storage::User::column<"id">("u").to_expr());
        if (paginated)
            query.limit(pageSize).offset(skip);
        const auto rs = co_await db.query(query);

        UserPageDataDto result;
        result.set<"total">(static_cast<ruvia::int64>(total))
            .set<"page">(static_cast<ruvia::int64>(page))
            .set<"pageSize">(static_cast<ruvia::int64>(pageSize))
            .set<"totalPages">(static_cast<ruvia::int64>(
                paginated && pageSize > 0 ? (total + pageSize - 1) / pageSize : 1
            ));

        auto& list = result.ensure<"list">();
        for (const auto& row : rs) {
            auto& item = list.emplace_back();
            const auto userId = fillUserItem(item, row);
            co_await attachRoles(c, item, userId);
        }
        co_return result;
    }

    ruvia::task<ruvia::array<UserOptionDto>> listOptions(ruvia::context& c,
                                                         std::optional<std::string_view> keyword) {
        auto db = c.db();
        ruvia::db_query query;
        query
            .select({storage::User::column<"id">().to_expr(),
                     storage::User::column<"username">().to_expr(),
                     storage::User::column<"nickname">().to_expr(),
                     storage::User::column<"phone">().to_expr(),
                     storage::User::column<"email">().to_expr()})
            .from(storage::User::table_name())
            .where(storage::User::column<"deleted_at">().is_null());
        if (keyword && !keyword->empty()) {
            const auto like = "%" + ruvia::escape_like_pattern(*keyword) + "%";
            query.and_where(storage::User::column<"username">().like(like) ||
                            storage::User::column<"nickname">().like(like) ||
                            storage::User::column<"phone">().like(like) ||
                            storage::User::column<"email">().like(like));
        }
        query.order_by(storage::User::column<"id">().to_expr());
        const auto rs = co_await db.query(query);
        ruvia::array<UserOptionDto> out;
        for (const auto& row : rs) {
            auto& item = out.emplace_back();
            item.set<"id">(
                static_cast<ruvia::int64>(std::stoll(std::string(row[0].value().value_or(""))))
            );
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

    ruvia::task<UserItemDto> getById(ruvia::context& c, std::int64_t id) {
        auto db = c.db();
        const auto rs = co_await db.query(
            ruvia::db_query{}
                .select({storage::User::column<"id">("u").to_expr(),
                         storage::User::column<"username">("u").to_expr(),
                         storage::User::column<"nickname">("u").to_expr(),
                         storage::User::column<"phone">("u").to_expr(),
                         storage::User::column<"email">("u").to_expr(),
                         storage::User::column<"dept_id">("u").to_expr(),
                         storage::User::column<"status">("u").to_expr(),
                         storage::Department::column<"name">("d").to_expr()})
                .from(storage::User::table_name(), "u")
                .join(ruvia::db_join_type::left, storage::Department::table_name(),
                      storage::User::column<"dept_id">("u").to_expr() ==
                          storage::Department::column<"id">("d").to_expr(),
                      "d")
                .where(storage::User::column<"id">("u").to_expr() == id &&
                       storage::User::column<"deleted_at">("u").to_expr().is_null())
                .limit(1)
        );
        if (rs.empty())
            service::common::throwAppError(UserError::USER_NOT_FOUND);

        UserItemDto item;
        const auto userId = fillUserItem(item, rs.front());
        co_await attachRoles(c, item, userId);
        co_return item;
    }

    ruvia::task<void> create(ruvia::context& c, const CreateUserBody& body) {
        auto db = c.db();
        const std::string username(body.get<"username">().view());
        const std::string phone =
            body.get<"phone">() ? std::string(body.get<"phone">()->view()) : std::string{};
        const std::string email =
            body.get<"email">() ? std::string(body.get<"email">()->view()) : std::string{};
        const auto existing = co_await db.query(
            ruvia::db_query{}
                .select(storage::User::column<"id">().to_expr())
                .from(storage::User::table_name())
                .where(storage::User::column<"username">().to_expr() == username &&
                       storage::User::column<"deleted_at">().to_expr().is_null())
                .limit(1)
        );
        if (!existing.empty())
            service::common::throwAppError(UserError::USERNAME_EXISTS);

        if (!phone.empty())
            co_await checkPhoneUnique(c, phone, 0);
        if (!email.empty())
            co_await checkEmailUnique(c, email, 0);

        if (body.get<"roleIds">().empty())
            service::common::throwAppError(UserError::ROLE_REQUIRED);

        const auto hash = service::utils::hashPassword(body.get<"password">().view());
        auto tx = co_await db.begin_transaction();
        const auto rs = co_await tx.execute(
            ruvia::db_query{}
                .insert_into(storage::User::table_name(),
                             {"username", "password_hash", "nickname", "phone", "email", "dept_id",
                              "status", "created_at", "updated_at"})
                .values({db_expr::value(ruvia::db_value{username}),
                         db_expr::value(ruvia::db_value{hash}),
                         db_expr::value(body.get<"nickname">()
                                            ? ruvia::db_value{body.get<"nickname">()->view()}
                                            : ruvia::db_value{nullptr}),
                         db_expr::value(!phone.empty() ? ruvia::db_value{phone}
                                                       : ruvia::db_value{nullptr}),
                         db_expr::value(!email.empty() ? ruvia::db_value{email}
                                                       : ruvia::db_value{nullptr}),
                         db_expr::value(
                             body.get<"deptId">()
                                 ? ruvia::db_value{static_cast<std::int64_t>(*body.get<"deptId">())}
                                 : ruvia::db_value{nullptr}
                         ),
                         db_expr::value(ruvia::db_value{body.get<"status">()
                                                            ? body.get<"status">()->view()
                                                            : std::string_view{"enabled"}}),
                         db_expr::now(), db_expr::now()})
        );
        const std::int64_t userId = static_cast<std::int64_t>(rs.last_insert_id().value_or(0));

        for (const auto roleId : body.get<"roleIds">()) {
            storage::UserRole relation;
            relation.set<"user_id">(userId);
            relation.set<"role_id">(static_cast<std::int64_t>(roleId));
            const std::array<ruvia::borrowed_text, 1> duplicateKey{"user_id"};
            (void)co_await tx.repository<storage::UserRole>().upsert(
                relation, {.update_columns_ = duplicateKey, .any_unique_key_ = true}
            );
        }
        co_await tx.commit();
        co_return;
    }

    ruvia::task<void> update(ruvia::context& c, std::int64_t id, const UpdateUserBody& body) {
        auto db = c.db();
        const auto rs =
            co_await db.query(ruvia::db_query{}
                                  .select(storage::User::column<"username">().to_expr())
                                  .from(storage::User::table_name())
                                  .where(storage::User::column<"id">().to_expr() == id &&
                                         storage::User::column<"deleted_at">().to_expr().is_null())
                                  .limit(1));
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

        std::vector<ruvia::db_assignment> changes;
        auto append = [&](std::string_view col, ruvia::db_value value) {
            changes.emplace_back(col, db_expr::value(std::move(value)));
        };
        if (body.get<"nickname">())
            append("nickname", ruvia::db_value{body.get<"nickname">()->view()});
        if (body.get<"phone">())
            append("phone", ruvia::db_value{phone});
        if (body.get<"email">())
            append("email", ruvia::db_value{email});
        if (body.get<"deptId">())
            append("dept_id", ruvia::db_value{static_cast<std::int64_t>(*body.get<"deptId">())});
        if (body.get<"status">())
            append("status", ruvia::db_value{body.get<"status">()->view()});
        if (body.get<"password">() && !body.get<"password">()->empty()) {
            const auto passwordHash = service::utils::hashPassword(body.get<"password">()->view());
            append("password_hash", ruvia::db_value{passwordHash});
        }
        auto tx = co_await db.begin_transaction();
        if (!changes.empty()) {
            (void)co_await tx.execute(ruvia::db_query{}
                                          .update(storage::User::table_name())
                                          .set(changes)
                                          .set("updated_at", db_expr::now())
                                          .where(storage::User::column<"id">().to_expr() == id));
        }

        if (body.get<"roleIds">()) {
            (void)co_await tx.execute(
                ruvia::db_query{}
                    .delete_from(storage::UserRole::table_name())
                    .where(storage::UserRole::column<"user_id">().to_expr() == id)
            );
            for (const auto roleId : *body.get<"roleIds">()) {
                storage::UserRole relation;
                relation.set<"user_id">(id);
                relation.set<"role_id">(static_cast<std::int64_t>(roleId));
                const std::array<ruvia::borrowed_text, 1> duplicateKey{"user_id"};
                (void)co_await tx.repository<storage::UserRole>().upsert(
                    relation, {.update_columns_ = duplicateKey, .any_unique_key_ = true}
                );
            }
        }
        co_await tx.commit();
        if (body.get<"roleIds">())
            service::middleware::permissionService().clearUserCache(id);
        co_return;
    }

    ruvia::task<void> remove(ruvia::context& c, std::int64_t id) {
        auto db = c.db();
        const auto rs =
            co_await db.query(ruvia::db_query{}
                                  .select(storage::User::column<"username">().to_expr())
                                  .from(storage::User::table_name())
                                  .where(storage::User::column<"id">().to_expr() == id &&
                                         storage::User::column<"deleted_at">().to_expr().is_null())
                                  .limit(1));
        if (rs.empty())
            service::common::throwAppError(UserError::USER_NOT_FOUND);
        if (std::string(rs.front()[0].value().value_or("")) == "admin") {
            service::common::throwAppError(UserError::ADMIN_DELETE_PROTECTED);
        }
        (void)co_await db.execute(ruvia::db_query{}
                                      .update(storage::User::table_name())
                                      .set("deleted_at", db_expr::now())
                                      .where(storage::User::column<"id">().to_expr() == id));
        service::middleware::permissionService().clearUserCache(id);
        co_return;
    }

  private:
    UserService() = default;

    template <typename Row>
    static std::int64_t fillUserItem(UserItemDto& item, const Row& row) {
        const std::int64_t userId = std::stoll(std::string(row[0].value().value_or("")));
        item.set<"id">(static_cast<ruvia::int64>(userId));
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
                static_cast<ruvia::int64>(std::stoll(std::string(row[5].value().value_or(""))))
            );
        }
        if (row[7].value().has_value())
            item.set<"deptName">(row[7].value().value_or(""));
        return userId;
    }

    ruvia::task<void> attachRoles(ruvia::context& c, UserItemDto& item, std::int64_t userId) {
        auto db = c.db();
        const auto roles = co_await db.query(
            ruvia::db_query{}
                .select({storage::Role::column<"id">("r").to_expr(),
                         storage::Role::column<"name">("r").to_expr(),
                         storage::Role::column<"code">("r").to_expr()})
                .from(storage::Role::table_name(), "r")
                .join(ruvia::db_join_type::inner, storage::UserRole::table_name(),
                      storage::Role::column<"id">("r").to_expr() ==
                          storage::UserRole::column<"role_id">("ur").to_expr(),
                      "ur")
                .where(storage::UserRole::column<"user_id">("ur").to_expr() == userId &&
                       storage::Role::column<"deleted_at">("r").to_expr().is_null())
        );
        auto& roleList = item.ensure<"roles">();
        for (const auto& rrow : roles) {
            auto& role = roleList.emplace_back();
            role.set<"id">(
                static_cast<ruvia::int64>(std::stoll(std::string(rrow[0].value().value_or(""))))
            );
            role.set<"name">(rrow[1].value().value_or(""));
            role.set<"code">(rrow[2].value().value_or(""));
        }
        co_return;
    }

    ruvia::task<void> checkPhoneUnique(ruvia::context& c, const std::string& phone,
                                       std::int64_t excludeId) {
        if (phone.empty())
            co_return;
        auto db = c.db();
        const auto rs = co_await db.query(
            ruvia::db_query{}
                .select(storage::User::column<"id">().to_expr())
                .from(storage::User::table_name())
                .where(storage::User::column<"phone">().to_expr() == phone &&
                       storage::User::column<"deleted_at">().to_expr().is_null() &&
                       storage::User::column<"id">().to_expr() != excludeId)
                .limit(1)
        );
        if (!rs.empty())
            service::common::throwAppError(UserError::PHONE_EXISTS);
        co_return;
    }

    ruvia::task<void> checkEmailUnique(ruvia::context& c, const std::string& email,
                                       std::int64_t excludeId) {
        if (email.empty())
            co_return;
        auto db = c.db();
        const auto rs = co_await db.query(
            ruvia::db_query{}
                .select(storage::User::column<"id">().to_expr())
                .from(storage::User::table_name())
                .where(storage::User::column<"email">().to_expr() == email &&
                       storage::User::column<"deleted_at">().to_expr().is_null() &&
                       storage::User::column<"id">().to_expr() != excludeId)
                .limit(1)
        );
        if (!rs.empty())
            service::common::throwAppError(UserError::EMAIL_EXISTS);
        co_return;
    }
};

inline UserService& userService() { return UserService::instance(); }

} // namespace service::user
