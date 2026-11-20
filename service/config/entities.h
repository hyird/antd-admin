#pragma once

#include <cstdint>
#include <string>

#include <db_entity.h>

namespace service::storage {

RUVIA_DB_ENTITY(
    Department, "sys_dept",
    RUVIA_DB_COLUMN(id, std::int64_t, ruvia::db_column_options{.primary_key_ = true}),
    RUVIA_DB_COLUMN(name, std::string),
    RUVIA_DB_COLUMN(code, std::string, ruvia::db_column_options{.nullable_ = true}),
    RUVIA_DB_COLUMN(parent_id, std::int64_t, ruvia::db_column_options{.nullable_ = true}),
    RUVIA_DB_COLUMN(order, std::int64_t),
    RUVIA_DB_COLUMN(leader_id, std::int64_t, ruvia::db_column_options{.nullable_ = true}),
    RUVIA_DB_COLUMN(status, std::string), RUVIA_DB_COLUMN(created_at, std::string),
    RUVIA_DB_COLUMN(updated_at, std::string),
    RUVIA_DB_COLUMN(deleted_at, std::string, ruvia::db_column_options{.nullable_ = true})
)

RUVIA_DB_ENTITY(
    Menu, "sys_menu",
    RUVIA_DB_COLUMN(id, std::int64_t, ruvia::db_column_options{.primary_key_ = true}),
    RUVIA_DB_COLUMN(name, std::string),
    RUVIA_DB_COLUMN(path, std::string, ruvia::db_column_options{.nullable_ = true}),
    RUVIA_DB_COLUMN(icon, std::string, ruvia::db_column_options{.nullable_ = true}),
    RUVIA_DB_COLUMN(parent_id, std::int64_t, ruvia::db_column_options{.nullable_ = true}),
    RUVIA_DB_COLUMN(order, std::int64_t), RUVIA_DB_COLUMN(type, std::string),
    RUVIA_DB_COLUMN(component, std::string, ruvia::db_column_options{.nullable_ = true}),
    RUVIA_DB_COLUMN(status, std::string),
    RUVIA_DB_COLUMN(permission_code, std::string, ruvia::db_column_options{.nullable_ = true}),
    RUVIA_DB_COLUMN(is_default, bool), RUVIA_DB_COLUMN(created_at, std::string),
    RUVIA_DB_COLUMN(updated_at, std::string),
    RUVIA_DB_COLUMN(deleted_at, std::string, ruvia::db_column_options{.nullable_ = true})
)

RUVIA_DB_ENTITY(Role, "sys_role",
                RUVIA_DB_COLUMN(id, std::int64_t, ruvia::db_column_options{.primary_key_ = true}),
                RUVIA_DB_COLUMN(code, std::string), RUVIA_DB_COLUMN(name, std::string),
                RUVIA_DB_COLUMN(description, std::string,
                                ruvia::db_column_options{.nullable_ = true}),
                RUVIA_DB_COLUMN(status, std::string), RUVIA_DB_COLUMN(created_at, std::string),
                RUVIA_DB_COLUMN(updated_at, std::string),
                RUVIA_DB_COLUMN(deleted_at, std::string,
                                ruvia::db_column_options{.nullable_ = true}))

RUVIA_DB_ENTITY(RoleMenu, "sys_role_menu",
                RUVIA_DB_COLUMN(role_id, std::int64_t,
                                ruvia::db_column_options{.primary_key_ = true}),
                RUVIA_DB_COLUMN(menu_id, std::int64_t,
                                ruvia::db_column_options{.primary_key_ = true}))

RUVIA_DB_ENTITY(User, "sys_user",
                RUVIA_DB_COLUMN(id, std::int64_t, ruvia::db_column_options{.primary_key_ = true}),
                RUVIA_DB_COLUMN(username, std::string), RUVIA_DB_COLUMN(password_hash, std::string),
                RUVIA_DB_COLUMN(nickname, std::string, ruvia::db_column_options{.nullable_ = true}),
                RUVIA_DB_COLUMN(phone, std::string, ruvia::db_column_options{.nullable_ = true}),
                RUVIA_DB_COLUMN(email, std::string, ruvia::db_column_options{.nullable_ = true}),
                RUVIA_DB_COLUMN(dept_id, std::int64_t, ruvia::db_column_options{.nullable_ = true}),
                RUVIA_DB_COLUMN(status, std::string), RUVIA_DB_COLUMN(created_at, std::string),
                RUVIA_DB_COLUMN(updated_at, std::string),
                RUVIA_DB_COLUMN(deleted_at, std::string,
                                ruvia::db_column_options{.nullable_ = true}))

RUVIA_DB_ENTITY(UserRole, "sys_user_role",
                RUVIA_DB_COLUMN(user_id, std::int64_t,
                                ruvia::db_column_options{.primary_key_ = true}),
                RUVIA_DB_COLUMN(role_id, std::int64_t,
                                ruvia::db_column_options{.primary_key_ = true}))

} // namespace service::storage
