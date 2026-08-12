#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <ruvia/core/Task.h>
#include <ruvia/web/db/Db.h>
#include <ruvia/web/Context.h>

#include "service/common/http.h"
#include "service/modules/system/dept/dept.error.h"
#include "service/modules/system/dept/dept.types.h"

namespace service::dept {

class DeptService {
  public:
    static DeptService& instance() {
        static DeptService svc;
        return svc;
    }

    ruvia::Task<DeptPageDataDto> list(ruvia::Context& c, std::int64_t page, std::int64_t pageSize,
                                      std::int64_t skip, const std::optional<std::string>& keyword,
                                      bool paginated, std::optional<std::string_view> status,
                                      std::optional<std::int64_t> parentId) {
        auto db = c.db();

        std::string where = " FROM sys_dept d WHERE d.deleted_at IS NULL";
        std::vector<ruvia::DbValue> params;
        if (keyword) {
            where += " AND (d.name LIKE ? OR d.code LIKE ?)";
            const std::string like = "%" + service::common::escapeLikePattern(*keyword) + "%";
            params.emplace_back(like);
            params.emplace_back(like);
        }
        if (status && !status->empty()) {
            where += " AND d.status = ?";
            params.emplace_back(*status);
        }
        if (parentId) {
            const auto parentValue = *parentId;
            if (parentValue <= 0) {
                where += " AND d.parent_id IS NULL";
            } else {
                where += " AND d.parent_id = ?";
                params.emplace_back(parentValue);
            }
        }

        const auto countRs = co_await db.query("SELECT COUNT(*)" + where, params);
        const std::int64_t total =
            countRs.empty() ? 0 : std::stoll(std::string(countRs.front()[0].value().value_or("")));

        std::string sql =
            "SELECT d.id, d.name, d.code, d.parent_id, d.`order`, d.leader_id, d.status" + where +
            " ORDER BY d.`order` ASC, d.id ASC";
        if (paginated) {
            sql += " LIMIT " + std::to_string(pageSize) + " OFFSET " + std::to_string(skip);
        }
        const auto rs = co_await db.query(sql, params);

        DeptPageDataDto result(c);
        result.set<"total">(static_cast<ruvia::Int64>(total))
            .set<"page">(static_cast<ruvia::Int64>(page))
            .set<"pageSize">(static_cast<ruvia::Int64>(pageSize))
            .set<"totalPages">(static_cast<ruvia::Int64>(
                paginated && pageSize > 0 ? (total + pageSize - 1) / pageSize : 1));

        auto& list = result.ensure<"list">();
        for (const auto& row : rs) {
            auto& item = list.emplace(c);
            fillDeptDto(item, rowToRecord(row));
        }
        co_return result;
    }

    ruvia::Task<ruvia::BoxedArray<DeptDto>> getTree(ruvia::Context& c,
                                                    std::optional<std::string_view> status) {
        auto db = c.db();
        std::string sql =
            "SELECT id, name, code, parent_id, `order`, leader_id, status FROM sys_dept "
            "WHERE deleted_at IS NULL";
        std::vector<ruvia::DbValue> params;
        if (status && !status->empty()) {
            sql += " AND status = ?";
            params.emplace_back(*status);
        }
        sql += " ORDER BY `order` ASC, id ASC";
        const auto rs = co_await db.query(sql, params);
        co_return buildTree(c, rowsToRecords(rs));
    }

    ruvia::Task<DeptDto> getById(ruvia::Context& c, std::int64_t id) {
        auto db = c.db();
        const auto rs = co_await db.query(
            "SELECT id, name, code, parent_id, `order`, leader_id, status FROM sys_dept "
            "WHERE id = ? AND deleted_at IS NULL LIMIT 1",
            service::common::dbParams(ruvia::DbValue{id}));
        if (rs.empty())
            service::common::throwAppError(DeptError::NOT_FOUND);

        DeptDto out(c);
        fillDeptDto(out, rowToRecord(rs.front()));
        co_return out;
    }

    ruvia::Task<void> create(ruvia::Context& c, const CreateDeptBody& body) {
        auto db = c.db();
        const auto code = body.get<"code">()
                              ? std::optional<std::string>(std::string(body.get<"code">()->view()))
                              : std::nullopt;
        if (code) {
            const auto exist = co_await db.query(
                "SELECT id FROM sys_dept WHERE code = ? AND deleted_at IS NULL LIMIT 1",
                service::common::dbParams(ruvia::DbValue{*code}));
            if (!exist.empty())
                service::common::throwAppError(DeptError::CODE_EXISTS);
        }
        if (body.get<"parentId">()) {
            const auto parent = co_await db.query(
                "SELECT id FROM sys_dept WHERE id = ? AND deleted_at IS NULL LIMIT 1",
                service::common::dbParams(
                    ruvia::DbValue{static_cast<std::int64_t>(*body.get<"parentId">())}));
            if (parent.empty())
                service::common::throwAppError(DeptError::NOT_FOUND);
        }

        (void)co_await db.execute(
            "INSERT INTO sys_dept (name, code, parent_id, `order`, leader_id, status, "
            "                          created_at, updated_at) "
            "VALUES (?, ?, ?, ?, ?, ?, NOW(), NOW())",
            service::common::dbParams(
                ruvia::DbValue{body.get<"name">()->view()},
                code ? ruvia::DbValue{*code} : ruvia::DbValue{nullptr},
                body.get<"parentId">()
                    ? ruvia::DbValue{static_cast<std::int64_t>(*body.get<"parentId">())}
                    : ruvia::DbValue{nullptr},
                ruvia::DbValue{body.get<"sortOrder">()
                                   ? static_cast<std::int64_t>(*body.get<"sortOrder">())
                                   : 0},
                body.get<"leaderId">()
                    ? ruvia::DbValue{static_cast<std::int64_t>(*body.get<"leaderId">())}
                    : ruvia::DbValue{nullptr},
                ruvia::DbValue{body.get<"status">() ? body.get<"status">()->view()
                                                    : std::string_view{"enabled"}}));
        co_return;
    }

    ruvia::Task<void> update(ruvia::Context& c, std::int64_t id, const UpdateDeptBody& body) {
        auto db = c.db();
        const auto rs = co_await db.query(
            "SELECT id, code FROM sys_dept WHERE id = ? AND deleted_at IS NULL LIMIT 1",
            service::common::dbParams(ruvia::DbValue{id}));
        if (rs.empty())
            service::common::throwAppError(DeptError::NOT_FOUND);
        const std::string currentCode = !rs.front()[1].value().has_value()
                                            ? std::string{}
                                            : std::string(rs.front()[1].value().value_or(""));

        const auto code = body.get<"code">()
                              ? std::optional<std::string>(std::string(body.get<"code">()->view()))
                              : std::nullopt;
        if (code && *code != currentCode) {
            const auto exist = co_await db.query(
                "SELECT id FROM sys_dept WHERE code = ? AND id != ? AND deleted_at IS NULL LIMIT 1",
                service::common::dbParams(ruvia::DbValue{*code}, ruvia::DbValue{id}));
            if (!exist.empty())
                service::common::throwAppError(DeptError::CODE_EXISTS);
        }

        if (body.get<"parentId">()) {
            const auto parentId = static_cast<std::int64_t>(*body.get<"parentId">());
            if (parentId == id)
                service::common::throwAppError(DeptError::PARENT_SELF);
            if (parentId > 0) {
                const auto parent = co_await db.query(
                    "SELECT id FROM sys_dept WHERE id = ? AND deleted_at IS NULL LIMIT 1",
                    service::common::dbParams(ruvia::DbValue{parentId}));
                if (parent.empty())
                    service::common::throwAppError(DeptError::NOT_FOUND);
                if (co_await isAncestorDescendant(c, id, parentId)) {
                    service::common::throwAppError(DeptError::PARENT_IS_CHILD);
                }
            }
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
        if (body.get<"name">())
            append("name", ruvia::DbValue{body.get<"name">()->view()});
        if (code)
            append("code", ruvia::DbValue{*code});
        if (body.get<"parentId">())
            append("parent_id", ruvia::DbValue{static_cast<std::int64_t>(*body.get<"parentId">())});
        if (body.get<"sortOrder">())
            append("`order`", ruvia::DbValue{static_cast<std::int64_t>(*body.get<"sortOrder">())});
        if (body.get<"leaderId">())
            append("leader_id", ruvia::DbValue{static_cast<std::int64_t>(*body.get<"leaderId">())});
        if (body.get<"status">())
            append("status", ruvia::DbValue{body.get<"status">()->view()});

        if (!set.empty()) {
            params.emplace_back(ruvia::DbValue{id});
            (void)co_await db.execute(
                "UPDATE sys_dept SET " + set + ", updated_at = NOW() WHERE id = ?", params);
        }
        co_return;
    }

    ruvia::Task<void> remove(ruvia::Context& c, std::int64_t id) {
        auto db = c.db();
        const auto rs =
            co_await db.query("SELECT id FROM sys_dept WHERE id = ? AND deleted_at IS NULL LIMIT 1",
                              service::common::dbParams(ruvia::DbValue{id}));
        if (rs.empty())
            service::common::throwAppError(DeptError::NOT_FOUND);

        const auto child = co_await db.query(
            "SELECT COUNT(*) FROM sys_dept WHERE parent_id = ? AND deleted_at IS NULL",
            service::common::dbParams(ruvia::DbValue{id}));
        if (std::stoll(std::string(child.front()[0].value().value_or(""))) > 0) {
            service::common::throwAppError(DeptError::HAS_CHILDREN);
        }

        const auto users = co_await db.query(
            "SELECT COUNT(*) FROM sys_user WHERE dept_id = ? AND deleted_at IS NULL",
            service::common::dbParams(ruvia::DbValue{id}));
        if (std::stoll(std::string(users.front()[0].value().value_or(""))) > 0) {
            service::common::throwAppError(DeptError::HAS_USERS);
        }

        (void)co_await db.execute("UPDATE sys_dept SET deleted_at = NOW() WHERE id = ?",
                                  service::common::dbParams(ruvia::DbValue{id}));
        co_return;
    }

  private:
    struct DeptRecord {
        std::int64_t id{0};
        std::string_view name;
        std::optional<std::string_view> code;
        std::optional<std::int64_t> parent_id;
        std::int64_t sort_order{0};
        std::optional<std::int64_t> leader_id;
        std::string_view status;
    };

    DeptService() = default;

    ruvia::Task<bool> isAncestorDescendant(ruvia::Context& c, std::int64_t ancestor,
                                           std::int64_t candidate) {
        auto db = c.db();
        const auto rs =
            co_await db.query("SELECT id, parent_id FROM sys_dept WHERE deleted_at IS NULL");
        std::unordered_map<std::int64_t, std::vector<std::int64_t>> children;
        for (const auto& row : rs) {
            if (!row[1].value().has_value())
                continue;
            const std::int64_t id = std::stoll(std::string(row[0].value().value_or("")));
            const std::int64_t parent = std::stoll(std::string(row[1].value().value_or("")));
            children[parent].push_back(id);
        }
        std::unordered_set<std::int64_t> seen{ancestor};
        std::vector<std::int64_t> stack{ancestor};
        while (!stack.empty()) {
            const auto current = stack.back();
            stack.pop_back();
            for (const auto child : children[current]) {
                if (child == candidate)
                    co_return true;
                if (seen.insert(child).second)
                    stack.push_back(child);
            }
        }
        co_return false;
    }

    template <typename Row> static DeptRecord rowToRecord(const Row& row) {
        DeptRecord item;
        item.id = std::stoll(std::string(row[0].value().value_or("")));
        item.name = row[1].value().value_or("");
        if (row[2].value().has_value())
            item.code = row[2].value().value_or("");
        if (row[3].value().has_value())
            item.parent_id = std::stoll(std::string(row[3].value().value_or("")));
        item.sort_order = std::stoll(std::string(row[4].value().value_or("")));
        if (row[5].value().has_value())
            item.leader_id = std::stoll(std::string(row[5].value().value_or("")));
        item.status = row[6].value().value_or("");
        return item;
    }

    template <typename Rows> static std::vector<DeptRecord> rowsToRecords(const Rows& rows) {
        std::vector<DeptRecord> out;
        out.reserve(rows.size());
        for (const auto& row : rows)
            out.push_back(rowToRecord(row));
        return out;
    }

    static void fillDeptDto(DeptDto& item, const DeptRecord& record) {
        item.set<"id">(static_cast<ruvia::Int64>(record.id))
            .set<"sortOrder">(static_cast<ruvia::Int64>(record.sort_order));
        item.set<"name">(record.name);
        item.set<"status">(record.status);
        if (record.code)
            item.set<"code">(*record.code);
        if (record.parent_id)
            item.set<"parentId">(static_cast<ruvia::Int64>(*record.parent_id));
        if (record.leader_id)
            item.set<"leaderId">(static_cast<ruvia::Int64>(*record.leader_id));
    }

    static ruvia::BoxedArray<DeptDto> buildFlatList(ruvia::Context& c,
                                                    const std::vector<DeptRecord>& records) {
        ruvia::BoxedArray<DeptDto> out(c.resource());
        for (const auto& record : records) {
            auto& item = out.emplace(c);
            fillDeptDto(item, record);
        }
        return out;
    }

    static void
    appendNode(ruvia::Context& c, ruvia::BoxedArray<DeptDto>& out, const DeptRecord& record,
               const std::unordered_map<std::int64_t, std::vector<const DeptRecord*>>& children) {
        auto& item = out.emplace(c);
        fillDeptDto(item, record);

        const auto it = children.find(record.id);
        if (it == children.end())
            return;
        auto& childList = item.ensure<"children">();
        for (const auto* child : it->second) {
            appendNode(c, childList, *child, children);
        }
    }

    static ruvia::BoxedArray<DeptDto> buildTree(ruvia::Context& c,
                                                const std::vector<DeptRecord>& records) {
        ruvia::BoxedArray<DeptDto> out(c.resource());
        std::unordered_set<std::int64_t> ids;
        ids.reserve(records.size());
        for (const auto& record : records)
            ids.insert(record.id);

        std::vector<const DeptRecord*> roots;
        std::unordered_map<std::int64_t, std::vector<const DeptRecord*>> children;
        for (const auto& record : records) {
            if (record.parent_id && ids.contains(*record.parent_id)) {
                children[*record.parent_id].push_back(&record);
            } else {
                roots.push_back(&record);
            }
        }

        for (const auto* root : roots)
            appendNode(c, out, *root, children);
        return out;
    }
};

inline DeptService& deptService() { return DeptService::instance(); }

} // namespace service::dept
