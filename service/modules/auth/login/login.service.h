#pragma once

#include <chrono>
#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

#include <task.h>
#include <db_executor.h>
#include <context.h>

#include <db_query.h>

#include "service/common/http.h"
#include "service/config/entities.h"
#include "service/common/types.h"
#include "service/middleware/permission.h"
#include "service/modules/auth/login/login.error.h"
#include "service/modules/auth/login/login.schema.h"
#include "service/modules/auth/login/login.types.h"
#include "service/modules/system/menu/menu.service.h"
#include "service/utils/password.h"
#include "service/utils/jwt.h"

namespace service::auth {

using ruvia::db_expr;

class RateLimitService {
  public:
    static RateLimitService& instance() {
        static RateLimitService svc;
        return svc;
    }

    bool isLocked(const std::string& username) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = records_.find(username);
        if (it == records_.end())
            return false;
        const auto now = std::chrono::steady_clock::now();
        if (it->second.expires_at < now) {
            records_.erase(it);
            return false;
        }
        return it->second.count >= kMaxAttempts;
    }

    int recordFailure(const std::string& username) {
        std::lock_guard<std::mutex> lock(mutex_);
        const auto now = std::chrono::steady_clock::now();
        auto it = records_.find(username);
        if (it == records_.end() || it->second.expires_at < now) {
            records_[username] = {1, now + kLockDuration};
            return 1;
        }
        ++it->second.count;
        return it->second.count;
    }

    void clearFailure(const std::string& username) {
        std::lock_guard<std::mutex> lock(mutex_);
        records_.erase(username);
    }

    std::int64_t remainingLockSeconds(const std::string& username) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = records_.find(username);
        if (it == records_.end())
            return 0;
        const auto now = std::chrono::steady_clock::now();
        if (it->second.expires_at < now || it->second.count < kMaxAttempts)
            return 0;
        return std::chrono::duration_cast<std::chrono::seconds>(it->second.expires_at - now)
            .count();
    }

  private:
    struct Record {
        int count{0};
        std::chrono::steady_clock::time_point expires_at;
    };
    static constexpr int kMaxAttempts = 5;
    static constexpr std::chrono::minutes kLockDuration{15};

    std::mutex mutex_;
    std::unordered_map<std::string, Record> records_;
};

inline RateLimitService& rateLimitService() { return RateLimitService::instance(); }

class AuthService {
  public:
    static AuthService& instance() {
        static AuthService svc;
        return svc;
    }

    ruvia::task<LoginResultDto> login(ruvia::context& c, const LoginBody& req) {
        const std::string username(req.get<"username">().view());
        const std::string password(req.get<"password">().view());

        if (rateLimitService().isLocked(username)) {
            const auto remaining = rateLimitService().remainingLockSeconds(username);
            const auto minutes = (remaining + 59) / 60;
            service::common::throwAppError(
                AuthError::TOO_MANY_ATTEMPTS.code,
                "登录失败次数过多，请" + std::to_string(minutes) + "分钟后再试", 429
            );
        }

        auto db = c.db();
        const auto users = co_await db.query(
            ruvia::db_query{}
                .select({storage::User::column<"id">().to_expr(),
                         storage::User::column<"username">().to_expr(),
                         storage::User::column<"password_hash">().to_expr(),
                         storage::User::column<"nickname">().to_expr(),
                         storage::User::column<"status">().to_expr()})
                .from(storage::User::table_name())
                .where(storage::User::column<"username">().to_expr() == username &&
                       storage::User::column<"deleted_at">().to_expr().is_null())
                .limit(1)
        );
        if (users.empty()) {
            const int failureCount = rateLimitService().recordFailure(username);
            const int remaining = 5 - failureCount;
            if (remaining > 0) {
                service::common::throwAppError(
                    AuthError::PASSWORD_INCORRECT.code,
                    "用户名或密码错误，还剩" + std::to_string(remaining) + "次尝试机会", 401
                );
            }
            service::common::throwAppError(AuthError::TOO_MANY_ATTEMPTS);
        }

        const auto& row = users.front();
        const std::int64_t userId = row[0].value().value_or("").empty()
                                        ? 0
                                        : std::stoll(std::string(row[0].value().value_or("")));
        const std::string passwordHash(row[2].value().value_or(""));
        const std::string nickname =
            !row[3].value().has_value() ? std::string{} : std::string(row[3].value().value_or(""));
        const std::string status(row[4].value().value_or(""));

        if (!service::utils::comparePassword(password, passwordHash)) {
            const int failureCount = rateLimitService().recordFailure(username);
            const int remaining = 5 - failureCount;
            if (remaining > 0) {
                service::common::throwAppError(
                    AuthError::PASSWORD_INCORRECT.code,
                    "用户名或密码错误，还剩" + std::to_string(remaining) + "次尝试机会", 401
                );
            }
            service::common::throwAppError(AuthError::TOO_MANY_ATTEMPTS);
        }
        if (status == "disabled")
            service::common::throwAppError(AuthError::USER_DISABLED);

        rateLimitService().clearFailure(username);

        const service::core::JwtPayload payload{userId, username};
        auto user = co_await buildUserInfo(c, userId, username, nickname, status);
        LoginResultDto result;
        result.set<"token">(service::utils::signAccessToken(payload))
            .set<"refreshToken">(service::utils::signRefreshToken(payload))
            .set<"user">(std::move(user));
        co_return result;
    }

    ruvia::task<LoginResultDto> refresh(ruvia::context& c, const RefreshBody& req) {
        const std::string refreshToken(req.get<"refreshToken">().view());
        if (refreshToken.empty())
            service::common::throwAppError(AuthError::UNAUTHORIZED);
        service::core::JwtPayload payload;
        try {
            payload = service::utils::verifyRefreshToken(refreshToken);
        } catch (...) {
            service::common::throwAppError(AuthError::TOKEN_INVALID);
        }

        auto db = c.db();
        const auto users = co_await db.query(
            ruvia::db_query{}
                .select({storage::User::column<"id">().to_expr(),
                         storage::User::column<"username">().to_expr(),
                         storage::User::column<"nickname">().to_expr(),
                         storage::User::column<"status">().to_expr()})
                .from(storage::User::table_name())
                .where(storage::User::column<"id">().to_expr() == payload.user_id &&
                       storage::User::column<"deleted_at">().to_expr().is_null())
                .limit(1)
        );
        if (users.empty())
            service::common::throwAppError(AuthError::USER_NOT_FOUND);
        const auto& row = users.front();
        const std::int64_t userId = std::stoll(std::string(row[0].value().value_or("")));
        const std::string username(row[1].value().value_or(""));
        const std::string nickname =
            !row[2].value().has_value() ? std::string{} : std::string(row[2].value().value_or(""));
        const std::string status(row[3].value().value_or(""));
        if (status == "disabled")
            service::common::throwAppError(AuthError::USER_DISABLED);

        const service::core::JwtPayload next{userId, username};
        auto user = co_await buildUserInfo(c, userId, username, nickname, status);
        LoginResultDto result;
        result.set<"token">(service::utils::signAccessToken(next))
            .set<"refreshToken">(service::utils::signRefreshToken(next))
            .set<"user">(std::move(user));
        co_return result;
    }

    ruvia::task<AuthUserInfoDto> getCurrentUser(ruvia::context& c, std::int64_t userId) {
        auto db = c.db();
        const auto users =
            co_await db.query(ruvia::db_query{}
                                  .select({storage::User::column<"username">().to_expr(),
                                           storage::User::column<"nickname">().to_expr(),
                                           storage::User::column<"status">().to_expr()})
                                  .from(storage::User::table_name())
                                  .where(storage::User::column<"id">().to_expr() == userId &&
                                         storage::User::column<"deleted_at">().to_expr().is_null())
                                  .limit(1));
        if (users.empty())
            service::common::throwAppError(AuthError::USER_NOT_FOUND);
        const auto& row = users.front();
        const std::string username(row[0].value().value_or(""));
        const std::string nickname =
            !row[1].value().has_value() ? std::string{} : std::string(row[1].value().value_or(""));
        const std::string status(row[2].value().value_or(""));
        if (status == "disabled")
            service::common::throwAppError(AuthError::USER_DISABLED);
        co_return co_await buildUserInfo(c, userId, username, nickname, status);
    }

  private:
    AuthService() = default;

    ruvia::task<bool> loadUserRoles(ruvia::context& c, AuthUserInfoDto& info, std::int64_t userId) {
        auto db = c.db();
        const auto rs = co_await db.query(
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
        bool isSuperadmin = false;
        auto& roles = info.ensure<"roles">();
        for (const auto& row : rs) {
            auto& role = roles.emplace_back();
            role.set<"id">(
                static_cast<ruvia::int64>(std::stoll(std::string(row[0].value().value_or(""))))
            );

            const auto code = row[2].value().value_or("");
            role.set<"name">(row[1].value().value_or(""));
            role.set<"code">(code);
            if (code == service::common::kSuperAdminRoleCode)
                isSuperadmin = true;
        }
        co_return isSuperadmin;
    }

    ruvia::task<ruvia::boxed_array<menu::MenuDto>> getUserRoleMenus(ruvia::context& c,
                                                                    std::int64_t userId) {
        auto db = c.db();
        const auto rs = co_await db.query(
            ruvia::db_query{}
                .select({storage::Menu::column<"id">("m").to_expr(),
                         storage::Menu::column<"name">("m").to_expr(),
                         storage::Menu::column<"path">("m").to_expr(),
                         storage::Menu::column<"icon">("m").to_expr(),
                         storage::Menu::column<"parent_id">("m").to_expr(),
                         storage::Menu::column<"order">("m").to_expr(),
                         storage::Menu::column<"type">("m").to_expr(),
                         storage::Menu::column<"component">("m").to_expr(),
                         storage::Menu::column<"status">("m").to_expr(),
                         storage::Menu::column<"permission_code">("m").to_expr(),
                         storage::Menu::column<"is_default">("m").to_expr()})
                .from(storage::Menu::table_name(), "m")
                .distinct()
                .join(ruvia::db_join_type::inner, storage::RoleMenu::table_name(),
                      storage::Menu::column<"id">("m").to_expr() ==
                          storage::RoleMenu::column<"menu_id">("rm").to_expr(),
                      "rm")
                .join(ruvia::db_join_type::inner, storage::UserRole::table_name(),
                      storage::RoleMenu::column<"role_id">("rm").to_expr() ==
                          storage::UserRole::column<"role_id">("ur").to_expr(),
                      "ur")
                .join(ruvia::db_join_type::inner, storage::Role::table_name(),
                      storage::UserRole::column<"role_id">("ur").to_expr() ==
                          storage::Role::column<"id">("r").to_expr(),
                      "r")
                .where(storage::UserRole::column<"user_id">("ur").to_expr() == userId &&
                       storage::Role::column<"deleted_at">("r").to_expr().is_null() &&
                       storage::Role::column<"status">("r").to_expr() == "enabled" &&
                       storage::Menu::column<"deleted_at">("m").to_expr().is_null() &&
                       storage::Menu::column<"status">("m").to_expr() == "enabled")
                .order_by(storage::Menu::column<"order">("m").to_expr())
                .add_order_by(storage::Menu::column<"id">("m").to_expr())
        );
        co_return menu::MenuService::flatFromRows(rs);
    }

    ruvia::task<AuthUserInfoDto> buildUserInfo(ruvia::context& c, std::int64_t userId,
                                               const std::string& username,
                                               const std::string& nickname,
                                               const std::string& status) {
        AuthUserInfoDto info;
        info.set<"id">(static_cast<ruvia::int64>(userId))
            .set<"username">(username)
            .set<"nickname">(nickname)
            .set<"status">(status);
        const bool isSuperadmin = co_await loadUserRoles(c, info, userId);
        if (isSuperadmin) {
            auto menus = co_await menu::menuService().listEnabledFlat(c);
            info.set<"menus">(std::move(menus));
        } else {
            auto menus = co_await getUserRoleMenus(c, userId);
            info.set<"menus">(std::move(menus));
        }
        co_return info;
    }
};

inline AuthService& authService() { return AuthService::instance(); }

} // namespace service::auth
