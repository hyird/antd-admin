#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <cyra/app/Task.h>
#include <cyra/db/Db.h>
#include <cyra/http/Context.h>

#include "service/common/http.h"
#include "service/middleware/permission.h"
#include "service/modules/system/menu/menu.error.h"
#include "service/modules/system/menu/menu.types.h"

namespace service::modules::system::menu {

class MenuService {
public:
    static MenuService& instance() {
        static MenuService svc;
        return svc;
    }

    template <typename Rows>
    static cyra::List<MenuDto> treeFromRows(cyra::Context& c, const Rows& rows) {
        return buildTree(c, rowsToRecords(rows));
    }

    cyra::Task<cyra::List<MenuDto>> listAll(cyra::Context& c, std::optional<std::string> keyword) {
        auto db = c.db();
        std::string sql =
            "SELECT id, name, path, icon, parent_id, `order`, type, component, status, "
            "       permission_code, is_default FROM sys_menu WHERE deleted_at IS NULL";
        std::vector<cyra::DbValue> params;
        if (keyword && !keyword->empty()) {
            sql += " AND (name LIKE ? OR path LIKE ?)";
            const std::string like = "%" + *keyword + "%";
            params.emplace_back(like);
            params.emplace_back(like);
        }
        sql += " ORDER BY `order` ASC, id ASC";
        const auto rs = co_await db.query(sql, params);
        co_return buildFlatList(c, rowsToRecords(rs.rows()));
    }

    cyra::Task<cyra::List<MenuDto>> getTree(cyra::Context& c, std::optional<std::string> status) {
        auto db = c.db();
        std::string sql =
            "SELECT id, name, path, icon, parent_id, `order`, type, component, status, "
            "       permission_code, is_default FROM sys_menu WHERE deleted_at IS NULL";
        std::vector<cyra::DbValue> params;
        if (status) {
            sql += " AND status = ?";
            params.emplace_back(*status);
        }
        sql += " ORDER BY `order` ASC, id ASC";
        const auto rs = co_await db.query(sql, params);
        co_return buildTree(c, rowsToRecords(rs.rows()));
    }

    cyra::Task<MenuDto> getDetail(cyra::Context& c, std::int64_t id) {
        auto db = c.db();
        const auto rs = co_await db.query(
            "SELECT id, name, path, icon, parent_id, `order`, type, component, status, "
            "       permission_code, is_default FROM sys_menu "
            "WHERE id = ? AND deleted_at IS NULL LIMIT 1",
            {cyra::DbValue{id}});
        if (rs.rows().empty()) service::common::throwAppError(MenuError::MENU_NOT_FOUND);

        MenuDto out(c);
        fillMenuDto(out, rowToRecord(rs.rows().front()));
        co_return out;
    }

    cyra::Task<void> create(cyra::Context& c, const CreateMenuBody& body) {
        auto db = c.db();
        const std::string type = body.type() ? std::string(body.type()->view()) : "menu";

        std::string parentType;
        if (body.parentId()) {
            const auto prs = co_await db.query(
                "SELECT type FROM sys_menu WHERE id = ? AND deleted_at IS NULL",
                {cyra::DbValue{static_cast<std::int64_t>(*body.parentId())}});
            if (prs.rows().empty()) service::common::throwAppError(MenuError::MENU_PARENT_NOT_FOUND);
            parentType = std::string(prs.rows().front()[0].text());
            checkParentChildType(parentType, type);
        }
        const bool isDefault = body.isDefault() && static_cast<bool>(*body.isDefault());
        if (isDefault && type != "page") {
            service::common::throwAppError(MenuError::DEFAULT_MUST_BE_PAGE);
        }

        if (isDefault) {
            (void)co_await db.execute("UPDATE sys_menu SET is_default = 0 WHERE is_default = 1");
        }

        (void)co_await db.execute(
            "INSERT INTO sys_menu (name, path, icon, component, parent_id, `order`, type, status, "
            "                     permission_code, is_default, created_at, updated_at) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, NOW(), NOW())",
            {cyra::DbValue{std::string(body.name()->view())},
             body.path() ? cyra::DbValue{std::string(body.path()->view())} : cyra::DbValue{nullptr},
             body.icon() ? cyra::DbValue{std::string(body.icon()->view())} : cyra::DbValue{nullptr},
             body.component() ? cyra::DbValue{std::string(body.component()->view())}
                              : cyra::DbValue{nullptr},
             body.parentId() ? cyra::DbValue{static_cast<std::int64_t>(*body.parentId())}
                             : cyra::DbValue{nullptr},
             cyra::DbValue{body.sortOrder() ? static_cast<std::int64_t>(*body.sortOrder()) : 0},
             cyra::DbValue{type},
             cyra::DbValue{body.status() ? std::string(body.status()->view()) : "enabled"},
             body.permissionCode() ? cyra::DbValue{std::string(body.permissionCode()->view())}
                                   : cyra::DbValue{nullptr},
             cyra::DbValue{static_cast<std::int64_t>(isDefault ? 1 : 0)}});
        service::middleware::permissionService().clearAllCache();
        co_return;
    }

    cyra::Task<void> update(cyra::Context& c, std::int64_t id, const UpdateMenuBody& body) {
        auto db = c.db();
        const auto rs = co_await db.query(
            "SELECT type, parent_id FROM sys_menu WHERE id = ? AND deleted_at IS NULL LIMIT 1",
            {cyra::DbValue{id}});
        if (rs.rows().empty()) service::common::throwAppError(MenuError::MENU_NOT_FOUND);
        const std::string currentType(rs.rows().front()[0].text());

        const auto type = body.type() ? std::optional<std::string>(std::string(body.type()->view()))
                                      : std::nullopt;
        const std::string newType = type.value_or(currentType);
        const auto newParentId = body.parentId()
                                     ? std::optional<std::int64_t>(static_cast<std::int64_t>(*body.parentId()))
                                     : std::nullopt;
        if (newParentId && *newParentId == id) {
            service::common::throwAppError(MenuError::MENU_PARENT_SELF);
        }
        if (newParentId && *newParentId > 0) {
            const auto prs = co_await db.query(
                "SELECT type FROM sys_menu WHERE id = ? AND deleted_at IS NULL",
                {cyra::DbValue{*newParentId}});
            if (prs.rows().empty()) service::common::throwAppError(MenuError::MENU_PARENT_NOT_FOUND);
            checkParentChildType(std::string(prs.rows().front()[0].text()), newType);
        }
        if (body.isDefault() && static_cast<bool>(*body.isDefault()) && type && *type != "page") {
            service::common::throwAppError(MenuError::DEFAULT_MUST_BE_PAGE);
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
        if (body.path()) append("path", cyra::DbValue{std::string(body.path()->view())});
        if (body.icon()) append("icon", cyra::DbValue{std::string(body.icon()->view())});
        if (body.component()) append("component", cyra::DbValue{std::string(body.component()->view())});
        if (newParentId) append("parent_id", cyra::DbValue{*newParentId});
        if (body.sortOrder()) append("`order`", cyra::DbValue{static_cast<std::int64_t>(*body.sortOrder())});
        if (type) append("type", cyra::DbValue{*type});
        if (body.status()) append("status", cyra::DbValue{std::string(body.status()->view())});
        if (body.permissionCode()) {
            append("permission_code", cyra::DbValue{std::string(body.permissionCode()->view())});
        }
        if (body.isDefault()) {
            const bool nextDefault = static_cast<bool>(*body.isDefault());
            if (nextDefault) {
                (void)co_await db.execute("UPDATE sys_menu SET is_default = 0 WHERE is_default = 1");
            }
            append("is_default", cyra::DbValue{static_cast<std::int64_t>(nextDefault ? 1 : 0)});
        }

        if (!set.empty()) {
            params.emplace_back(cyra::DbValue{id});
            (void)co_await db.execute("UPDATE sys_menu SET " + set + ", updated_at = NOW() WHERE id = ?",
                                    params);
        }
        service::middleware::permissionService().clearAllCache();
        co_return;
    }

    cyra::Task<void> remove(cyra::Context& c, std::int64_t id) {
        auto db = c.db();
        const auto rs = co_await db.query(
            "SELECT id FROM sys_menu WHERE id = ? AND deleted_at IS NULL LIMIT 1",
            {cyra::DbValue{id}});
        if (rs.rows().empty()) service::common::throwAppError(MenuError::MENU_NOT_FOUND);

        const auto childRs = co_await db.query(
            "SELECT COUNT(*) FROM sys_menu WHERE parent_id = ? AND deleted_at IS NULL",
            {cyra::DbValue{id}});
        if (std::stoll(std::string(childRs.rows().front()[0].text())) > 0) {
            service::common::throwAppError(MenuError::MENU_HAS_CHILDREN);
        }

        (void)co_await db.execute("UPDATE sys_menu SET deleted_at = NOW() WHERE id = ?",
                                {cyra::DbValue{id}});
        service::middleware::permissionService().clearAllCache();
        co_return;
    }

    cyra::Task<void> reorder(cyra::Context& c, const ReorderMenuBody& body) {
        if (!body.items() || body.items()->empty()) co_return;
        auto db = c.db();
        for (const auto& item : *body.items()) {
            (void)co_await db.execute("UPDATE sys_menu SET `order` = ?, updated_at = NOW() WHERE id = ?",
                                    {cyra::DbValue{static_cast<std::int64_t>(*item.sortOrder())},
                                     cyra::DbValue{static_cast<std::int64_t>(*item.id())}});
        }
        service::middleware::permissionService().clearAllCache();
        co_return;
    }

    cyra::Task<int> batchCreateButtons(cyra::Context& c, const BatchCreateMenuButtonsBody& body) {
        auto db = c.db();
        const auto parentId = static_cast<std::int64_t>(*body.parentId());
        const auto prs = co_await db.query(
            "SELECT type FROM sys_menu WHERE id = ? AND deleted_at IS NULL",
            {cyra::DbValue{parentId}});
        if (prs.rows().empty()) service::common::throwAppError(MenuError::MENU_PARENT_NOT_FOUND);
        if (std::string(prs.rows().front()[0].text()) != "page") {
            service::common::throwAppError(MenuError::MENU_TYPE_INVALID);
        }

        int created = 0;
        for (const auto& item : *body.items()) {
            const std::string name(item.name()->view());
            const std::string permissionCode(item.permissionCode()->view());
            const auto exists = co_await db.query(
                "SELECT id FROM sys_menu WHERE parent_id = ? AND type = 'button' "
                "  AND permission_code = ? AND deleted_at IS NULL LIMIT 1",
                {cyra::DbValue{parentId}, cyra::DbValue{permissionCode}});
            if (!exists.rows().empty()) continue;
            const auto maxRs = co_await db.query(
                "SELECT COALESCE(MAX(`order`), 0) FROM sys_menu WHERE parent_id = ?",
                {cyra::DbValue{parentId}});
            const std::int64_t maxOrder =
                std::stoll(std::string(maxRs.rows().front()[0].text()));
            (void)co_await db.execute(
                "INSERT INTO sys_menu (name, parent_id, type, status, permission_code, `order`, "
                "                     created_at, updated_at) "
                "VALUES (?, ?, 'button', 'enabled', ?, ?, NOW(), NOW())",
                {cyra::DbValue{name}, cyra::DbValue{parentId},
                 cyra::DbValue{permissionCode}, cyra::DbValue{maxOrder + 1}});
            ++created;
        }
        service::middleware::permissionService().clearAllCache();
        co_return created;
    }

private:
    struct MenuRecord {
        std::int64_t id{0};
        std::string name;
        std::optional<std::string> path;
        std::optional<std::string> icon;
        std::optional<std::int64_t> parent_id;
        std::int64_t sort_order{0};
        std::string type;
        std::optional<std::string> component;
        std::string status;
        std::optional<std::string> permission_code;
        bool is_default{false};
    };

    MenuService() = default;

    static void checkParentChildType(const std::string& parentType, const std::string& childType) {
        if (parentType.empty()) return;
        if (parentType == "button") service::common::throwAppError(MenuError::MENU_TYPE_INVALID);
        if (parentType == "page" && childType != "button") {
            service::common::throwAppError(MenuError::MENU_TYPE_INVALID);
        }
    }

    template <typename Row>
    static MenuRecord rowToRecord(const Row& row) {
        MenuRecord item;
        item.id = std::stoll(std::string(row[0].text()));
        item.name = std::string(row[1].text());
        if (!row[2].isNull()) item.path = std::string(row[2].text());
        if (!row[3].isNull()) item.icon = std::string(row[3].text());
        if (!row[4].isNull()) item.parent_id = std::stoll(std::string(row[4].text()));
        item.sort_order = std::stoll(std::string(row[5].text()));
        item.type = std::string(row[6].text());
        if (!row[7].isNull()) item.component = std::string(row[7].text());
        item.status = std::string(row[8].text());
        if (!row[9].isNull()) item.permission_code = std::string(row[9].text());
        item.is_default = std::string(row[10].text()) == "1";
        return item;
    }

    template <typename Rows>
    static std::vector<MenuRecord> rowsToRecords(const Rows& rows) {
        std::vector<MenuRecord> out;
        out.reserve(rows.size());
        for (const auto& row : rows) out.push_back(rowToRecord(row));
        return out;
    }

    static void fillMenuDto(MenuDto& item, const MenuRecord& record) {
        item.id(static_cast<cyra::Int64>(record.id))
            .name(record.name)
            .sortOrder(static_cast<cyra::Int64>(record.sort_order))
            .type(record.type)
            .status(record.status)
            .isDefault(cyra::Bool{record.is_default});
        if (record.path) item.path(*record.path).fullPath(*record.path);
        if (record.icon) item.icon(*record.icon);
        if (record.parent_id) item.parentId(static_cast<cyra::Int64>(*record.parent_id));
        if (record.component) item.component(*record.component);
        if (record.permission_code) item.permissionCode(*record.permission_code);
    }

    static cyra::List<MenuDto> buildFlatList(cyra::Context& c, const std::vector<MenuRecord>& records) {
        cyra::List<MenuDto> out(c.resource());
        for (const auto& record : records) {
            auto& item = out.emplace(c);
            fillMenuDto(item, record);
        }
        return out;
    }

    static void appendNode(cyra::Context& c,
                           cyra::List<MenuDto>& out,
                           const MenuRecord& record,
                           const std::unordered_map<std::int64_t, std::vector<const MenuRecord*>>& children) {
        auto& item = out.emplace(c);
        fillMenuDto(item, record);

        const auto it = children.find(record.id);
        if (it == children.end()) return;
        auto& childList = item.childrenEnsure();
        for (const auto* child : it->second) {
            appendNode(c, childList, *child, children);
        }
    }

    static cyra::List<MenuDto> buildTree(cyra::Context& c, const std::vector<MenuRecord>& records) {
        cyra::List<MenuDto> out(c.resource());
        std::unordered_set<std::int64_t> ids;
        ids.reserve(records.size());
        for (const auto& record : records) ids.insert(record.id);

        std::vector<const MenuRecord*> roots;
        std::unordered_map<std::int64_t, std::vector<const MenuRecord*>> children;
        for (const auto& record : records) {
            if (record.parent_id && ids.contains(*record.parent_id)) {
                children[*record.parent_id].push_back(&record);
            } else {
                roots.push_back(&record);
            }
        }

        for (const auto* root : roots) appendNode(c, out, *root, children);
        return out;
    }
};

inline MenuService& menuService() { return MenuService::instance(); }

}  // namespace service::modules::system::menu
