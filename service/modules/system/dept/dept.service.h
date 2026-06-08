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
#include "service/modules/system/dept/dept.error.h"
#include "service/modules/system/dept/dept.types.h"

namespace service::dept {

class DeptService {
  public:
    static DeptService& instance() {
        static DeptService svc;
        return svc;
    }

    cyra::Task<DeptPageDataDto> list(cyra::Context& c, std::int64_t page, std::int64_t pageSize,
                                     std::int64_t skip, const std::optional<std::string>& keyword,
                                     bool paginated, std::optional<std::string_view> status,
                                     std::optional<std::int64_t> parentId) {
        auto db = c.db();

        std::string where = " FROM sys_dept d WHERE d.deleted_at IS NULL";
        std::vector<cyra::DbValue> params;
        if (keyword) {
            where += " AND (d.name LIKE ? OR d.code LIKE ?)";
            const std::string like = "%" + *keyword + "%";
            params.emplace_back(like);
            params.emplace_back(like);
        }
        if (status && !status->empty()) {
            where += " AND d.status = ?";
            params.emplace_back(std::string(*status));
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
            countRs.rows().empty() ? 0 : std::stoll(std::string(countRs.rows().front()[0].text()));

        std::string sql =
            "SELECT d.id, d.name, d.code, d.parent_id, d.`order`, d.leader_id, d.status" + where +
            " ORDER BY d.`order` ASC, d.id ASC";
        if (paginated) {
            sql += " LIMIT " + std::to_string(pageSize) + " OFFSET " + std::to_string(skip);
        }
        const auto rs = co_await db.query(sql, params);

        DeptPageDataDto result(c);
        result.total(static_cast<cyra::Int64>(total))
            .page(static_cast<cyra::Int64>(page))
            .pageSize(static_cast<cyra::Int64>(pageSize))
            .totalPages(static_cast<cyra::Int64>(
                paginated && pageSize > 0 ? (total + pageSize - 1) / pageSize : 1));

        auto& list = result.list().ensure();
        for (const auto& row : rs.rows()) {
            auto& item = list.emplace(c);
            fillDeptDto(item, rowToRecord(row));
        }
        co_return result;
    }

    cyra::Task<cyra::List<DeptDto>> getTree(cyra::Context& c,
                                            std::optional<std::string_view> status) {
        auto db = c.db();
        std::string sql =
            "SELECT id, name, code, parent_id, `order`, leader_id, status FROM sys_dept "
            "WHERE deleted_at IS NULL";
        std::vector<cyra::DbValue> params;
        if (status && !status->empty()) {
            sql += " AND status = ?";
            params.emplace_back(std::string(*status));
        }
        sql += " ORDER BY `order` ASC, id ASC";
        const auto rs = co_await db.query(sql, params);
        co_return buildTree(c, rowsToRecords(rs.rows()));
    }

    cyra::Task<DeptDto> getById(cyra::Context& c, std::int64_t id) {
        auto db = c.db();
        const auto rs = co_await db.query(
            "SELECT id, name, code, parent_id, `order`, leader_id, status FROM sys_dept "
            "WHERE id = ? AND deleted_at IS NULL LIMIT 1",
            {cyra::DbValue{id}});
        if (rs.rows().empty())
            service::common::throwAppError(DeptError::NOT_FOUND);

        DeptDto out(c);
        fillDeptDto(out, rowToRecord(rs.rows().front()));
        co_return out;
    }

    cyra::Task<void> create(cyra::Context& c, const CreateDeptBody& body) {
        auto db = c.db();
        const auto code = body.code() ? std::optional<std::string>(std::string(body.code()->view()))
                                      : std::nullopt;
        if (code) {
            const auto exist = co_await db.query(
                "SELECT id FROM sys_dept WHERE code = ? AND deleted_at IS NULL LIMIT 1",
                {cyra::DbValue{*code}});
            if (!exist.rows().empty())
                service::common::throwAppError(DeptError::CODE_EXISTS);
        }
        if (body.parentId()) {
            const auto parent = co_await db.query(
                "SELECT id FROM sys_dept WHERE id = ? AND deleted_at IS NULL LIMIT 1",
                {cyra::DbValue{static_cast<std::int64_t>(*body.parentId())}});
            if (parent.rows().empty())
                service::common::throwAppError(DeptError::NOT_FOUND);
        }

        (void)co_await db.execute(
            "INSERT INTO sys_dept (name, code, parent_id, `order`, leader_id, status, "
            "                          created_at, updated_at) "
            "VALUES (?, ?, ?, ?, ?, ?, NOW(), NOW())",
            {cyra::DbValue{std::string(body.name()->view())},
             code ? cyra::DbValue{*code} : cyra::DbValue{nullptr},
             body.parentId() ? cyra::DbValue{static_cast<std::int64_t>(*body.parentId())}
                             : cyra::DbValue{nullptr},
             cyra::DbValue{body.sortOrder() ? static_cast<std::int64_t>(*body.sortOrder()) : 0},
             body.leaderId() ? cyra::DbValue{static_cast<std::int64_t>(*body.leaderId())}
                             : cyra::DbValue{nullptr},
             cyra::DbValue{body.status() ? std::string(body.status()->view()) : "enabled"}});
        co_return;
    }

    cyra::Task<void> update(cyra::Context& c, std::int64_t id, const UpdateDeptBody& body) {
        auto db = c.db();
        const auto rs = co_await db.query(
            "SELECT id, code FROM sys_dept WHERE id = ? AND deleted_at IS NULL LIMIT 1",
            {cyra::DbValue{id}});
        if (rs.rows().empty())
            service::common::throwAppError(DeptError::NOT_FOUND);
        const std::string currentCode = rs.rows().front()[1].isNull()
                                            ? std::string{}
                                            : std::string(rs.rows().front()[1].text());

        const auto code = body.code() ? std::optional<std::string>(std::string(body.code()->view()))
                                      : std::nullopt;
        if (code && *code != currentCode) {
            const auto exist = co_await db.query(
                "SELECT id FROM sys_dept WHERE code = ? AND id != ? AND deleted_at IS NULL LIMIT 1",
                {cyra::DbValue{*code}, cyra::DbValue{id}});
            if (!exist.rows().empty())
                service::common::throwAppError(DeptError::CODE_EXISTS);
        }

        if (body.parentId()) {
            const auto parentId = static_cast<std::int64_t>(*body.parentId());
            if (parentId == id)
                service::common::throwAppError(DeptError::PARENT_SELF);
            if (parentId > 0) {
                const auto parent = co_await db.query(
                    "SELECT id FROM sys_dept WHERE id = ? AND deleted_at IS NULL LIMIT 1",
                    {cyra::DbValue{parentId}});
                if (parent.rows().empty())
                    service::common::throwAppError(DeptError::NOT_FOUND);
                if (co_await isAncestorDescendant(c, id, parentId)) {
                    service::common::throwAppError(DeptError::PARENT_IS_CHILD);
                }
            }
        }

        std::string set;
        std::vector<cyra::DbValue> params;
        auto append = [&](std::string_view col, cyra::DbValue value) {
            if (!set.empty())
                set += ", ";
            set.append(col);
            set += " = ?";
            params.emplace_back(std::move(value));
        };
        if (body.name())
            append("name", cyra::DbValue{std::string(body.name()->view())});
        if (code)
            append("code", cyra::DbValue{*code});
        if (body.parentId())
            append("parent_id", cyra::DbValue{static_cast<std::int64_t>(*body.parentId())});
        if (body.sortOrder())
            append("`order`", cyra::DbValue{static_cast<std::int64_t>(*body.sortOrder())});
        if (body.leaderId())
            append("leader_id", cyra::DbValue{static_cast<std::int64_t>(*body.leaderId())});
        if (body.status())
            append("status", cyra::DbValue{std::string(body.status()->view())});

        if (!set.empty()) {
            params.emplace_back(cyra::DbValue{id});
            (void)co_await db.execute(
                "UPDATE sys_dept SET " + set + ", updated_at = NOW() WHERE id = ?", params);
        }
        co_return;
    }

    cyra::Task<void> remove(cyra::Context& c, std::int64_t id) {
        auto db = c.db();
        const auto rs =
            co_await db.query("SELECT id FROM sys_dept WHERE id = ? AND deleted_at IS NULL LIMIT 1",
                              {cyra::DbValue{id}});
        if (rs.rows().empty())
            service::common::throwAppError(DeptError::NOT_FOUND);

        const auto child = co_await db.query(
            "SELECT COUNT(*) FROM sys_dept WHERE parent_id = ? AND deleted_at IS NULL",
            {cyra::DbValue{id}});
        if (std::stoll(std::string(child.rows().front()[0].text())) > 0) {
            service::common::throwAppError(DeptError::HAS_CHILDREN);
        }

        const auto users = co_await db.query(
            "SELECT COUNT(*) FROM sys_user WHERE dept_id = ? AND deleted_at IS NULL",
            {cyra::DbValue{id}});
        if (std::stoll(std::string(users.rows().front()[0].text())) > 0) {
            service::common::throwAppError(DeptError::HAS_USERS);
        }

        (void)co_await db.execute("UPDATE sys_dept SET deleted_at = NOW() WHERE id = ?",
                                  {cyra::DbValue{id}});
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

    cyra::Task<bool> isAncestorDescendant(cyra::Context& c, std::int64_t ancestor,
                                          std::int64_t candidate) {
        auto db = c.db();
        const auto rs =
            co_await db.query("SELECT id, parent_id FROM sys_dept WHERE deleted_at IS NULL");
        std::unordered_map<std::int64_t, std::vector<std::int64_t>> children;
        for (const auto& row : rs.rows()) {
            if (row[1].isNull())
                continue;
            const std::int64_t id = std::stoll(std::string(row[0].text()));
            const std::int64_t parent = std::stoll(std::string(row[1].text()));
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
        item.id = std::stoll(std::string(row[0].text()));
        item.name = row[1].text();
        if (!row[2].isNull())
            item.code = row[2].text();
        if (!row[3].isNull())
            item.parent_id = std::stoll(std::string(row[3].text()));
        item.sort_order = std::stoll(std::string(row[4].text()));
        if (!row[5].isNull())
            item.leader_id = std::stoll(std::string(row[5].text()));
        item.status = row[6].text();
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
        item.id(static_cast<cyra::Int64>(record.id))
            .sortOrder(static_cast<cyra::Int64>(record.sort_order));
        item.name().assignView(record.name);
        item.status().assignView(record.status);
        if (record.code)
            item.code().assignView(*record.code);
        if (record.parent_id)
            item.parentId(static_cast<cyra::Int64>(*record.parent_id));
        if (record.leader_id)
            item.leaderId(static_cast<cyra::Int64>(*record.leader_id));
    }

    static cyra::List<DeptDto> buildFlatList(cyra::Context& c,
                                             const std::vector<DeptRecord>& records) {
        cyra::List<DeptDto> out(c.resource());
        for (const auto& record : records) {
            auto& item = out.emplace(c);
            fillDeptDto(item, record);
        }
        return out;
    }

    static void
    appendNode(cyra::Context& c, cyra::List<DeptDto>& out, const DeptRecord& record,
               const std::unordered_map<std::int64_t, std::vector<const DeptRecord*>>& children) {
        auto& item = out.emplace(c);
        fillDeptDto(item, record);

        const auto it = children.find(record.id);
        if (it == children.end())
            return;
        auto& childList = item.children().ensure();
        for (const auto* child : it->second) {
            appendNode(c, childList, *child, children);
        }
    }

    static cyra::List<DeptDto> buildTree(cyra::Context& c, const std::vector<DeptRecord>& records) {
        cyra::List<DeptDto> out(c.resource());
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
