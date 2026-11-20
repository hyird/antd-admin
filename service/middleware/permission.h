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

#include <task.h>
#include <context.h>
#include <db_executor.h>

#include <db_query.h>

#include "service/common/http.h"
#include "service/config/entities.h"
#include "service/common/types.h"
#include "service/middleware/auth.h"

namespace service::middleware {

using ruvia::db_expr;

class PermissionService {
  public:
    static PermissionService& instance() {
        static PermissionService svc;
        return svc;
    }

    ruvia::task<bool> hasPermission(ruvia::context& c, std::int64_t userId, std::string_view code) {
        auto data = co_await loadUser(c, userId);
        co_return data.is_superadmin || data.permissions.count(std::string(code)) > 0;
    }

    ruvia::task<bool> hasAnyPermission(ruvia::context& c, std::int64_t userId,
                                       std::initializer_list<std::string_view> codes) {
        auto data = co_await loadUser(c, userId);
        if (data.is_superadmin)
            co_return true;
        for (auto code : codes) {
            if (data.permissions.count(std::string(code)))
                co_return true;
        }
        co_return false;
    }

    ruvia::task<bool> hasAllPermissions(ruvia::context& c, std::int64_t userId,
                                        std::initializer_list<std::string_view> codes) {
        auto data = co_await loadUser(c, userId);
        if (data.is_superadmin)
            co_return true;
        for (auto code : codes) {
            if (!data.permissions.count(std::string(code)))
                co_return false;
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

    ruvia::task<Snapshot> loadUser(ruvia::context& c, std::int64_t userId) {
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
            ruvia::db_query{}
                .select({storage::Role::column<"code">("r").to_expr(),
                         storage::Role::column<"status">("r").to_expr()})
                .from(storage::Role::table_name(), "r")
                .join(ruvia::db_join_type::inner, storage::UserRole::table_name(),
                      storage::Role::column<"id">("r").to_expr() ==
                          storage::UserRole::column<"role_id">("ur").to_expr(),
                      "ur")
                .where(storage::UserRole::column<"user_id">("ur").to_expr() ==
                           db_expr::value(userId) &&
                       storage::Role::column<"deleted_at">("r").to_expr().is_null())
        );
        for (const auto& row : roles) {
            if (row.size() < 2)
                continue;
            if (row[1].value().value_or("") == "enabled" &&
                row[0].value().value_or("") == service::common::kSuperAdminRoleCode) {
                snap.is_superadmin = true;
            }
        }
        if (snap.is_superadmin) {
            std::lock_guard<std::mutex> lock(mutex_);
            cache_[userId] = snap;
            co_return snap;
        }

        const auto perms = co_await db.query(
            ruvia::db_query{}
                .select(storage::Menu::column<"permission_code">("m").to_expr())
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
                .where(storage::UserRole::column<"user_id">("ur").to_expr() ==
                           db_expr::value(userId) &&
                       storage::Role::column<"deleted_at">("r").to_expr().is_null() &&
                       storage::Role::column<"status">("r").to_expr() == "enabled" &&
                       storage::Menu::column<"deleted_at">("m").to_expr().is_null() &&
                       storage::Menu::column<"status">("m").to_expr() == "enabled" &&
                       storage::Menu::column<"permission_code">("m").to_expr().is_not_null() &&
                       storage::Menu::column<"permission_code">("m").to_expr() != "")
        );
        for (const auto& row : perms) {
            if (row.empty() || !row[0].value().has_value())
                continue;
            snap.permissions.emplace(row[0].value().value_or(""));
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

inline ruvia::task<void> requirePermission(ruvia::context& c, std::string_view code) {
    const auto& jwt = currentUser(c);
    if (jwt.user_id <= 0) {
        service::common::throwAppError(service::common::kAuthUnauthorizedErrorCode, "未登录", 401);
    }
    const bool allowed = co_await permissionService().hasPermission(c, jwt.user_id, code);
    if (!allowed) {
        service::common::throwAppError(service::common::kAuthPermissionDeniedErrorCode, "无权限",
                                       403);
    }
}

inline ruvia::task<void> requireAnyPermission(ruvia::context& c,
                                              std::initializer_list<std::string_view> codes) {
    const auto& jwt = currentUser(c);
    if (jwt.user_id <= 0) {
        service::common::throwAppError(service::common::kAuthUnauthorizedErrorCode, "未登录", 401);
    }
    const bool allowed = co_await permissionService().hasAnyPermission(c, jwt.user_id, codes);
    if (!allowed) {
        service::common::throwAppError(service::common::kAuthPermissionDeniedErrorCode, "无权限",
                                       403);
    }
}

inline ruvia::task<void> requireAllPermissions(ruvia::context& c,
                                               std::initializer_list<std::string_view> codes) {
    const auto& jwt = currentUser(c);
    if (jwt.user_id <= 0) {
        service::common::throwAppError(service::common::kAuthUnauthorizedErrorCode, "未登录", 401);
    }
    const bool allowed = co_await permissionService().hasAllPermissions(c, jwt.user_id, codes);
    if (!allowed) {
        service::common::throwAppError(service::common::kAuthPermissionDeniedErrorCode, "无权限",
                                       403);
    }
}

} // namespace service::middleware
