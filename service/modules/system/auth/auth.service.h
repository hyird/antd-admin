#pragma once

#include <chrono>
#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

#include <cyra/app/Task.h>
#include <cyra/db/Db.h>
#include <cyra/http/Context.h>

#include "service/common/http.h"
#include "service/common/types.h"
#include "service/middleware/permission.h"
#include "service/modules/system/auth/auth.error.h"
#include "service/modules/system/auth/auth.types.h"
#include "service/modules/system/menu/menu.service.h"
#include "service/utils/password.h"
#include "service/utils/jwt.h"

namespace service::modules::system::auth {

class RateLimitService {
public:
    static RateLimitService& instance() {
        static RateLimitService svc;
        return svc;
    }

    bool isLocked(const std::string& username) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = records_.find(username);
        if (it == records_.end()) return false;
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
        if (it == records_.end()) return 0;
        const auto now = std::chrono::steady_clock::now();
        if (it->second.expires_at < now || it->second.count < kMaxAttempts) return 0;
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

    cyra::Task<LoginResult> login(cyra::Context& c, const LoginBody& req) {
        const std::string username(req.username()->view());
        const std::string password(req.password()->view());

        if (rateLimitService().isLocked(username)) {
            const auto remaining = rateLimitService().remainingLockSeconds(username);
            const auto minutes = (remaining + 59) / 60;
            service::common::throwAppError(
                AuthError::TOO_MANY_ATTEMPTS.code,
                "登录失败次数过多，请" + std::to_string(minutes) + "分钟后再试", 429);
        }

        auto db = c.db();
        const auto users = co_await db.query(
            "SELECT id, username, password_hash, nickname, status "
            "FROM sys_user WHERE username = ? AND deleted_at IS NULL LIMIT 1",
            {cyra::DbValue{username}});
        if (users.rows().empty()) {
            const int failureCount = rateLimitService().recordFailure(username);
            const int remaining = 5 - failureCount;
            if (remaining > 0) {
                service::common::throwAppError(
                    AuthError::PASSWORD_INCORRECT.code,
                    "用户名或密码错误，还剩" + std::to_string(remaining) + "次尝试机会", 401);
            }
            service::common::throwAppError(AuthError::TOO_MANY_ATTEMPTS);
        }

        const auto& row = users.rows().front();
        const std::int64_t userId = row[0].text().empty() ? 0
                                                          : std::stoll(std::string(row[0].text()));
        const std::string passwordHash(row[2].text());
        const std::string nickname = row[3].isNull() ? std::string{} : std::string(row[3].text());
        const std::string status(row[4].text());

        if (!service::utils::comparePassword(password, passwordHash)) {
            const int failureCount = rateLimitService().recordFailure(username);
            const int remaining = 5 - failureCount;
            if (remaining > 0) {
                service::common::throwAppError(
                    AuthError::PASSWORD_INCORRECT.code,
                    "用户名或密码错误，还剩" + std::to_string(remaining) + "次尝试机会", 401);
            }
            service::common::throwAppError(AuthError::TOO_MANY_ATTEMPTS);
        }
        if (status == "disabled") service::common::throwAppError(AuthError::USER_DISABLED);

        rateLimitService().clearFailure(username);

        const service::core::JwtPayload payload{userId, username, 0, 0};
        LoginResult result(c.resource());
        result.token = service::utils::signAccessToken(payload);
        result.refresh_token = service::utils::signRefreshToken(payload);
        result.user = co_await buildUserInfo(c, userId, username, nickname, status);
        co_return result;
    }

    cyra::Task<LoginResult> refresh(cyra::Context& c, const RefreshBody& req) {
        const std::string refreshToken(req.refreshToken()->view());
        if (refreshToken.empty()) service::common::throwAppError(AuthError::UNAUTHORIZED);
        service::core::JwtPayload payload;
        try {
            payload = service::utils::verifyRefreshToken(refreshToken);
        } catch (...) {
            service::common::throwAppError(AuthError::TOKEN_INVALID);
        }

        auto db = c.db();
        const auto users = co_await db.query(
            "SELECT id, username, nickname, status "
            "FROM sys_user WHERE id = ? AND deleted_at IS NULL LIMIT 1",
            {cyra::DbValue{payload.user_id}});
        if (users.rows().empty()) service::common::throwAppError(AuthError::USER_NOT_FOUND);
        const auto& row = users.rows().front();
        const std::int64_t userId = std::stoll(std::string(row[0].text()));
        const std::string username(row[1].text());
        const std::string nickname = row[2].isNull() ? std::string{} : std::string(row[2].text());
        const std::string status(row[3].text());
        if (status == "disabled") service::common::throwAppError(AuthError::USER_DISABLED);

        const service::core::JwtPayload next{userId, username, 0, 0};
        LoginResult result(c.resource());
        result.token = service::utils::signAccessToken(next);
        result.refresh_token = service::utils::signRefreshToken(next);
        result.user = co_await buildUserInfo(c, userId, username, nickname, status);
        co_return result;
    }

    cyra::Task<UserInfo> getCurrentUser(cyra::Context& c, std::int64_t userId) {
        auto db = c.db();
        const auto users = co_await db.query(
            "SELECT username, nickname, status FROM sys_user "
            "WHERE id = ? AND deleted_at IS NULL LIMIT 1",
            {cyra::DbValue{userId}});
        if (users.rows().empty()) service::common::throwAppError(AuthError::USER_NOT_FOUND);
        const auto& row = users.rows().front();
        const std::string username(row[0].text());
        const std::string nickname = row[1].isNull() ? std::string{} : std::string(row[1].text());
        const std::string status(row[2].text());
        if (status == "disabled") service::common::throwAppError(AuthError::USER_DISABLED);
        co_return co_await buildUserInfo(c, userId, username, nickname, status);
    }

private:
    AuthService() = default;

    cyra::Task<void> loadUserRoles(cyra::Context& c, UserInfo& info, std::int64_t userId) {
        auto db = c.db();
        const auto rs = co_await db.query(
            "SELECT r.id, r.name, r.code FROM sys_role r "
            "INNER JOIN sys_user_role ur ON r.id = ur.role_id "
            "WHERE ur.user_id = ? AND r.deleted_at IS NULL",
            {cyra::DbValue{userId}});
        for (const auto& row : rs.rows()) {
            auto& role = info.roles.emplace_back(c);
            role.id(static_cast<cyra::Int64>(std::stoll(std::string(row[0].text()))));

            const auto code = row[2].text();
            role.name().assignView(row[1].text());
            role.code().assignView(code);
            if (code == service::common::kSuperAdminRoleCode) info.is_superadmin = true;
        }
        co_return;
    }

    cyra::Task<cyra::List<menu::MenuDto>> getAllMenus(cyra::Context& c) {
        auto db = c.db();
        const auto rs = co_await db.query(
            "SELECT id, name, path, icon, parent_id, `order`, type, component, status, "
            "       permission_code, is_default FROM sys_menu "
            "WHERE deleted_at IS NULL AND status = 'enabled' "
            "ORDER BY `order` ASC, id ASC");
        co_return menu::MenuService::flatFromRows(c, rs.rows());
    }

    cyra::Task<cyra::List<menu::MenuDto>> getUserRoleMenus(cyra::Context& c, std::int64_t userId) {
        auto db = c.db();
        const auto rs = co_await db.query(
            "SELECT DISTINCT m.id, m.name, m.path, m.icon, m.parent_id, m.`order`, m.type, "
            "       m.component, m.status, m.permission_code FROM sys_menu m "
            "INNER JOIN sys_role_menu rm ON m.id = rm.menu_id "
            "INNER JOIN sys_user_role ur ON rm.role_id = ur.role_id "
            "INNER JOIN sys_role r ON ur.role_id = r.id "
            "WHERE ur.user_id = ? AND r.deleted_at IS NULL AND r.status = 'enabled' "
            "  AND m.deleted_at IS NULL AND m.status = 'enabled' "
            "ORDER BY m.`order` ASC, m.id ASC",
            {cyra::DbValue{userId}});
        co_return menu::MenuService::flatFromRows(c, rs.rows());
    }

    cyra::Task<UserInfo> buildUserInfo(cyra::Context& c,
                                       std::int64_t userId,
                                       const std::string& username,
                                       const std::string& nickname,
                                       const std::string& status) {
        UserInfo info(c.resource());
        info.id = userId;
        info.username = username;
        info.nickname = nickname;
        info.status = status;
        co_await loadUserRoles(c, info, userId);
        if (info.is_superadmin) {
            info.menus = co_await getAllMenus(c);
        } else {
            info.menus = co_await getUserRoleMenus(c, userId);
        }
        co_return info;
    }
};

inline AuthService& authService() { return AuthService::instance(); }

}  // namespace service::modules::system::auth
