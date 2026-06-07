#pragma once

#include <chrono>
#include <cstdint>
#include <initializer_list>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <cyra/app/Task.h>
#include <cyra/db/Db.h>
#include <cyra/http/Context.h>

#include "service/common/http.h"
#include "service/common/types.h"
#include "service/middleware/auth.h"

namespace service::middleware {

class PermissionService {
public:
    static PermissionService& instance() {
        static PermissionService svc;
        return svc;
    }

    cyra::Task<bool> hasPermission(cyra::Context& c, std::int64_t userId, std::string_view code) {
        auto data = co_await loadUser(c, userId);
        co_return data.is_superadmin || data.permissions.count(std::string(code)) > 0;
    }

    cyra::Task<bool> hasAnyPermission(cyra::Context& c,
                                      std::int64_t userId,
                                      std::initializer_list<std::string_view> codes) {
        auto data = co_await loadUser(c, userId);
        if (data.is_superadmin) co_return true;
        for (auto code : codes) {
            if (data.permissions.count(std::string(code))) co_return true;
        }
        co_return false;
    }

    cyra::Task<bool> hasAllPermissions(cyra::Context& c,
                                       std::int64_t userId,
                                       std::initializer_list<std::string_view> codes) {
        auto data = co_await loadUser(c, userId);
        if (data.is_superadmin) co_return true;
        for (auto code : codes) {
            if (!data.permissions.count(std::string(code))) co_return false;
        }
        co_return true;
    }

    void clearUserCache(std::int64_t userId) {
        std::lock_guard<std::mutex> lock(mutex_);
        cache_.erase(userId);
    }

    void clearAllCache() {
        std::lock_guard<std::mutex> lock(mutex_);
        cache_.clear();
    }

private:
    struct Snapshot {
        std::unordered_set<std::string> permissions;
        bool is_superadmin{false};
        std::chrono::steady_clock::time_point expire_at;
    };

    static constexpr std::chrono::seconds kTtl{60};

    cyra::Task<Snapshot> loadUser(cyra::Context& c, std::int64_t userId) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = cache_.find(userId);
            if (it != cache_.end() && it->second.expire_at > std::chrono::steady_clock::now()) {
                co_return it->second;
            }
        }

        Snapshot snap;
        snap.expire_at = std::chrono::steady_clock::now() + kTtl;

        auto db = c.db();
        const auto roles = co_await db.query(
            "SELECT r.code, r.status FROM sys_role r "
            "INNER JOIN sys_user_role ur ON r.id = ur.role_id "
            "WHERE ur.user_id = ? AND r.deleted_at IS NULL",
            {cyra::DbValue{userId}});
        for (const auto& row : roles.rows()) {
            if (row.size() < 2) continue;
            if (row[1].text() == "enabled" && row[0].text() == service::common::kSuperAdminRoleCode) {
                snap.is_superadmin = true;
            }
        }
        if (snap.is_superadmin) {
            std::lock_guard<std::mutex> lock(mutex_);
            cache_[userId] = snap;
            co_return snap;
        }

        const auto perms = co_await db.query(
            "SELECT DISTINCT m.permission_code FROM sys_menu m "
            "INNER JOIN sys_role_menu rm ON m.id = rm.menu_id "
            "INNER JOIN sys_user_role ur ON rm.role_id = ur.role_id "
            "INNER JOIN sys_role r ON ur.role_id = r.id "
            "WHERE ur.user_id = ? AND r.deleted_at IS NULL AND r.status = 'enabled' "
            "  AND m.deleted_at IS NULL AND m.status = 'enabled' "
            "  AND m.permission_code IS NOT NULL AND m.permission_code != ''",
            {cyra::DbValue{userId}});
        for (const auto& row : perms.rows()) {
            if (row.empty() || row[0].isNull()) continue;
            snap.permissions.emplace(row[0].text());
        }

        {
            std::lock_guard<std::mutex> lock(mutex_);
            cache_[userId] = snap;
        }
        co_return snap;
    }

    std::mutex mutex_;
    std::unordered_map<std::int64_t, Snapshot> cache_;
};

inline PermissionService& permissionService() { return PermissionService::instance(); }

inline cyra::Task<void> requirePermission(cyra::Context& c, std::string_view code) {
    const auto& jwt = currentUser(c);
    if (jwt.user_id <= 0) {
        service::common::throwAppError(service::common::kAuthUnauthorizedErrorCode, "未登录", 401);
    }
    const bool allowed = co_await permissionService().hasPermission(c, jwt.user_id, code);
    if (!allowed) {
        service::common::throwAppError(service::common::kAuthPermissionDeniedErrorCode, "无权限", 403);
    }
}

inline cyra::Task<void> requireAnyPermission(cyra::Context& c,
                                             std::initializer_list<std::string_view> codes) {
    const auto& jwt = currentUser(c);
    if (jwt.user_id <= 0) {
        service::common::throwAppError(service::common::kAuthUnauthorizedErrorCode, "未登录", 401);
    }
    const bool allowed = co_await permissionService().hasAnyPermission(c, jwt.user_id, codes);
    if (!allowed) {
        service::common::throwAppError(service::common::kAuthPermissionDeniedErrorCode, "无权限", 403);
    }
}

inline cyra::Task<void> requireAllPermissions(cyra::Context& c,
                                              std::initializer_list<std::string_view> codes) {
    const auto& jwt = currentUser(c);
    if (jwt.user_id <= 0) {
        service::common::throwAppError(service::common::kAuthUnauthorizedErrorCode, "未登录", 401);
    }
    const bool allowed = co_await permissionService().hasAllPermissions(c, jwt.user_id, codes);
    if (!allowed) {
        service::common::throwAppError(service::common::kAuthPermissionDeniedErrorCode, "无权限", 403);
    }
}

}  // namespace service::middleware
