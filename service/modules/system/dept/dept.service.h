#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <task.h>
#include <db_executor.h>
#include <context.h>

#include <db_query.h>

#include "service/common/http.h"
#include "service/config/entities.h"
#include "service/modules/system/dept/dept.error.h"
#include "service/modules/system/dept/dept.schema.h"
#include "service/modules/system/dept/dept.types.h"

namespace service::dept {

using ruvia::db_expr;

class DeptService {
  public:
    static DeptService& instance() {
        static DeptService svc;
        return svc;
    }

    ruvia::task<DeptPageDataDto> list(ruvia::context& c, std::int64_t page, std::int64_t pageSize,
                                      std::int64_t skip, const std::optional<std::string>& keyword,
                                      bool paginated, std::optional<std::string_view> status,
                                      std::optional<std::int64_t> parentId) {
        auto db = c.db();

        ruvia::db_query query;
        query.from(storage::Department::table_name(), "d")
            .where(storage::Department::column<"deleted_at">("d").is_null());
        if (keyword) {
            const auto like = "%" + ruvia::escape_like_pattern(*keyword) + "%";
            query.and_where(storage::Department::column<"name">("d").like(like) ||
                            storage::Department::column<"code">("d").like(like));
        }
        if (status && !status->empty())
            query.and_where(storage::Department::column<"status">("d").to_expr() == *status);
        if (parentId) {
            const auto parent = storage::Department::column<"parent_id">("d").to_expr();
            query.and_where(*parentId <= 0 ? parent.is_null() : parent == *parentId);
        }

        query.select(db_expr::count());
        const auto countRows = co_await db.query(query);
        const auto total = countRows.front()[0].as<std::int64_t>().value_or(0);

        query.select({storage::Department::column<"id">("d").to_expr(),
                      storage::Department::column<"name">("d").to_expr(),
                      storage::Department::column<"code">("d").to_expr(),
                      storage::Department::column<"parent_id">("d").to_expr(),
                      storage::Department::column<"order">("d").to_expr(),
                      storage::Department::column<"leader_id">("d").to_expr(),
                      storage::Department::column<"status">("d").to_expr()});
        query.order_by(storage::Department::column<"order">("d").to_expr());
        query.add_order_by(storage::Department::column<"id">("d").to_expr());
        if (paginated)
            query.limit(pageSize).offset(skip);
        const auto rs = co_await db.query(query);

        DeptPageDataDto result;
        result.set<"total">(static_cast<ruvia::int64>(total))
            .set<"page">(static_cast<ruvia::int64>(page))
            .set<"pageSize">(static_cast<ruvia::int64>(pageSize))
            .set<"totalPages">(static_cast<ruvia::int64>(
                paginated && pageSize > 0 ? (total + pageSize - 1) / pageSize : 1
            ));

        auto& list = result.ensure<"list">();
        for (const auto& row : rs) {
            auto& item = list.emplace_back();
            fillDeptDto(item, rowToRecord(row));
        }
        co_return result;
    }

    ruvia::task<ruvia::boxed_array<DeptDto>> getTree(ruvia::context& c,
                                                     std::optional<std::string_view> status) {
        auto db = c.db();
        ruvia::db_query query;
        query
            .select({storage::Department::column<"id">().to_expr(),
                     storage::Department::column<"name">().to_expr(),
                     storage::Department::column<"code">().to_expr(),
                     storage::Department::column<"parent_id">().to_expr(),
                     storage::Department::column<"order">().to_expr(),
                     storage::Department::column<"leader_id">().to_expr(),
                     storage::Department::column<"status">().to_expr()})
            .from(storage::Department::table_name())
            .where(storage::Department::column<"deleted_at">().is_null());
        if (status && !status->empty())
            query.and_where(storage::Department::column<"status">().to_expr() == *status);
        query.order_by(storage::Department::column<"order">().to_expr());
        query.add_order_by(storage::Department::column<"id">().to_expr());
        const auto rs = co_await db.query(query);
        co_return buildTree(rowsToRecords(rs));
    }

    ruvia::task<DeptDto> getById(ruvia::context& c, std::int64_t id) {
        auto db = c.db();
        const auto rs = co_await db.query(
            ruvia::db_query{}
                .select({storage::Department::column<"id">().to_expr(),
                         storage::Department::column<"name">().to_expr(),
                         storage::Department::column<"code">().to_expr(),
                         storage::Department::column<"parent_id">().to_expr(),
                         storage::Department::column<"order">().to_expr(),
                         storage::Department::column<"leader_id">().to_expr(),
                         storage::Department::column<"status">().to_expr()})
                .from(storage::Department::table_name())
                .where(storage::Department::column<"id">().to_expr() == id &&
                       storage::Department::column<"deleted_at">().to_expr().is_null())
                .limit(1)
        );
        if (rs.empty())
            service::common::throwAppError(DeptError::NOT_FOUND);

        DeptDto out;
        fillDeptDto(out, rowToRecord(rs.front()));
        co_return out;
    }

    ruvia::task<void> create(ruvia::context& c, const CreateDeptBody& body) {
        auto db = c.db();
        const auto code = body.get<"code">()
                              ? std::optional<std::string>(std::string(body.get<"code">()->view()))
                              : std::nullopt;
        if (code) {
            const auto exist = co_await db.query(
                ruvia::db_query{}
                    .select(storage::Department::column<"id">().to_expr())
                    .from(storage::Department::table_name())
                    .where(storage::Department::column<"code">().to_expr() == *code &&
                           storage::Department::column<"deleted_at">().to_expr().is_null())
                    .limit(1)
            );
            if (!exist.empty())
                service::common::throwAppError(DeptError::CODE_EXISTS);
        }
        if (body.get<"parentId">()) {
            const auto parent = co_await db.query(
                ruvia::db_query{}
                    .select(storage::Department::column<"id">().to_expr())
                    .from(storage::Department::table_name())
                    .where(storage::Department::column<"id">().to_expr() ==
                               static_cast<std::int64_t>(*body.get<"parentId">()) &&
                           storage::Department::column<"deleted_at">().to_expr().is_null())
                    .limit(1)
            );
            if (parent.empty())
                service::common::throwAppError(DeptError::NOT_FOUND);
        }

        (void)co_await db.execute(
            ruvia::db_query{}
                .insert_into(storage::Department::table_name(),
                             {"name", "code", "parent_id", "order", "leader_id", "status",
                              "created_at", "updated_at"})
                .values(
                    {db_expr::value(ruvia::db_value{body.get<"name">().view()}),
                     db_expr::value(code ? ruvia::db_value{*code} : ruvia::db_value{nullptr}),
                     db_expr::value(
                         body.get<"parentId">()
                             ? ruvia::db_value{static_cast<std::int64_t>(*body.get<"parentId">())}
                             : ruvia::db_value{nullptr}
                     ),
                     db_expr::value(
                         ruvia::db_value{body.get<"sortOrder">()
                                             ? static_cast<std::int64_t>(*body.get<"sortOrder">())
                                             : 0}
                     ),
                     db_expr::value(
                         body.get<"leaderId">()
                             ? ruvia::db_value{static_cast<std::int64_t>(*body.get<"leaderId">())}
                             : ruvia::db_value{nullptr}
                     ),
                     db_expr::value(ruvia::db_value{body.get<"status">()
                                                        ? body.get<"status">()->view()
                                                        : std::string_view{"enabled"}}),
                     db_expr::now(), db_expr::now()}
                )
        );
        co_return;
    }

    ruvia::task<void> update(ruvia::context& c, std::int64_t id, const UpdateDeptBody& body) {
        auto db = c.db();
        const auto rs = co_await db.query(
            ruvia::db_query{}
                .select({storage::Department::column<"id">().to_expr(),
                         storage::Department::column<"code">().to_expr()})
                .from(storage::Department::table_name())
                .where(storage::Department::column<"id">().to_expr() == id &&
                       storage::Department::column<"deleted_at">().to_expr().is_null())
                .limit(1)
        );
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
                ruvia::db_query{}
                    .select(storage::Department::column<"id">().to_expr())
                    .from(storage::Department::table_name())
                    .where(storage::Department::column<"code">().to_expr() == *code &&
                           storage::Department::column<"id">().to_expr() != id &&
                           storage::Department::column<"deleted_at">().to_expr().is_null())
                    .limit(1)
            );
            if (!exist.empty())
                service::common::throwAppError(DeptError::CODE_EXISTS);
        }

        if (body.get<"parentId">()) {
            const auto parentId = static_cast<std::int64_t>(*body.get<"parentId">());
            if (parentId == id)
                service::common::throwAppError(DeptError::PARENT_SELF);
            if (parentId > 0) {
                const auto parent = co_await db.query(
                    ruvia::db_query{}
                        .select(storage::Department::column<"id">().to_expr())
                        .from(storage::Department::table_name())
                        .where(storage::Department::column<"id">().to_expr() == parentId &&
                               storage::Department::column<"deleted_at">().to_expr().is_null())
                        .limit(1)
                );
                if (parent.empty())
                    service::common::throwAppError(DeptError::NOT_FOUND);
                if (co_await isAncestorDescendant(c, id, parentId)) {
                    service::common::throwAppError(DeptError::PARENT_IS_CHILD);
                }
            }
        }

        std::vector<ruvia::db_assignment> changes;
        auto append = [&](std::string_view col, ruvia::db_value value) {
            changes.emplace_back(col, db_expr::value(std::move(value)));
        };
        if (body.get<"name">())
            append("name", ruvia::db_value{body.get<"name">()->view()});
        if (code)
            append("code", ruvia::db_value{*code});
        if (body.get<"parentId">())
            append("parent_id",
                   ruvia::db_value{static_cast<std::int64_t>(*body.get<"parentId">())});
        if (body.get<"sortOrder">())
            append("order", ruvia::db_value{static_cast<std::int64_t>(*body.get<"sortOrder">())});
        if (body.get<"leaderId">())
            append("leader_id",
                   ruvia::db_value{static_cast<std::int64_t>(*body.get<"leaderId">())});
        if (body.get<"status">())
            append("status", ruvia::db_value{body.get<"status">()->view()});

        if (!changes.empty()) {
            (void)co_await db.execute(
                ruvia::db_query{}
                    .update(storage::Department::table_name())
                    .set(changes)
                    .set("updated_at", db_expr::now())
                    .where(storage::Department::column<"id">().to_expr() == id)
            );
        }
        co_return;
    }

    ruvia::task<void> remove(ruvia::context& c, std::int64_t id) {
        auto db = c.db();
        const auto rs = co_await db.query(
            ruvia::db_query{}
                .select(storage::Department::column<"id">().to_expr())
                .from(storage::Department::table_name())
                .where(storage::Department::column<"id">().to_expr() == id &&
                       storage::Department::column<"deleted_at">().to_expr().is_null())
                .limit(1)
        );
        if (rs.empty())
            service::common::throwAppError(DeptError::NOT_FOUND);

        const auto child = co_await db.query(
            ruvia::db_query{}
                .select(db_expr::count())
                .from(storage::Department::table_name())
                .where(storage::Department::column<"parent_id">().to_expr() == id &&
                       storage::Department::column<"deleted_at">().to_expr().is_null())
        );
        if (std::stoll(std::string(child.front()[0].value().value_or(""))) > 0) {
            service::common::throwAppError(DeptError::HAS_CHILDREN);
        }

        const auto users = co_await db.query(
            ruvia::db_query{}
                .select(db_expr::count())
                .from(storage::User::table_name())
                .where(storage::User::column<"dept_id">().to_expr() == id &&
                       storage::User::column<"deleted_at">().to_expr().is_null())
        );
        if (std::stoll(std::string(users.front()[0].value().value_or(""))) > 0) {
            service::common::throwAppError(DeptError::HAS_USERS);
        }

        (void)co_await db.execute(ruvia::db_query{}
                                      .update(storage::Department::table_name())
                                      .set("deleted_at", db_expr::now())
                                      .where(storage::Department::column<"id">().to_expr() == id));
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

    ruvia::task<bool> isAncestorDescendant(ruvia::context& c, std::int64_t ancestor,
                                           std::int64_t candidate) {
        auto db = c.db();
        const auto rs = co_await db.query(
            ruvia::db_query{}
                .select({storage::Department::column<"id">().to_expr(),
                         storage::Department::column<"parent_id">().to_expr()})
                .from(storage::Department::table_name())
                .where(storage::Department::column<"deleted_at">().to_expr().is_null())
        );
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

    template <typename Row>
    static DeptRecord rowToRecord(const Row& row) {
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

    template <typename Rows>
    static std::vector<DeptRecord> rowsToRecords(const Rows& rows) {
        std::vector<DeptRecord> out;
        out.reserve(rows.size());
        for (const auto& row : rows)
            out.push_back(rowToRecord(row));
        return out;
    }

    static void fillDeptDto(DeptDto& item, const DeptRecord& record) {
        item.set<"id">(static_cast<ruvia::int64>(record.id))
            .set<"sortOrder">(static_cast<ruvia::int64>(record.sort_order));
        item.set<"name">(record.name);
        item.set<"status">(record.status);
        if (record.code)
            item.set<"code">(*record.code);
        if (record.parent_id)
            item.set<"parentId">(static_cast<ruvia::int64>(*record.parent_id));
        if (record.leader_id)
            item.set<"leaderId">(static_cast<ruvia::int64>(*record.leader_id));
    }

    static void
    appendNode(ruvia::boxed_array<DeptDto>& out, const DeptRecord& record,
               const std::unordered_map<std::int64_t, std::vector<const DeptRecord*>>& children) {
        auto& item = out.emplace_back();
        fillDeptDto(item, record);

        const auto it = children.find(record.id);
        if (it == children.end())
            return;
        auto& childList = item.ensure<"children">();
        for (const auto* child : it->second) {
            appendNode(childList, *child, children);
        }
    }

    static ruvia::boxed_array<DeptDto> buildTree(const std::vector<DeptRecord>& records) {
        ruvia::boxed_array<DeptDto> out;
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
            appendNode(out, *root, children);
        return out;
    }
};

inline DeptService& deptService() { return DeptService::instance(); }

} // namespace service::dept
