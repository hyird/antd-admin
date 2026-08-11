#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

#include <ruvia/web/Model.h>

#include "service/modules/system/role/role.types.h"

namespace service::user {

inline bool isPhoneNumber(std::string_view value) {
    if (value.size() != 11 || value[0] != '1' || value[1] < '3' || value[1] > '9')
        return false;
    for (const char ch : value.substr(2)) {
        if (ch < '0' || ch > '9')
            return false;
    }
    return true;
}

struct CreateUserBody final {
    RUVIA_OPTIONAL_FIELD(username, ruvia::String);
    RUVIA_OPTIONAL_FIELD(password, ruvia::String);
    RUVIA_OPTIONAL_FIELD(nickname, ruvia::String);
    RUVIA_OPTIONAL_FIELD(phone, ruvia::String);
    RUVIA_OPTIONAL_FIELD(email, ruvia::String);
    RUVIA_OPTIONAL_FIELD_NAME("dept_id", deptId, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD(status, ruvia::String);
    RUVIA_OPTIONAL_FIELD_NAME("role_ids", roleIds, ruvia::Array<ruvia::Int64>);
    RUVIA_MODEL(CreateUserBody, username, password, nickname, phone, email, deptId, status,
                roleIds);
};

struct UpdateUserBody final {
    RUVIA_OPTIONAL_FIELD(nickname, ruvia::String);
    RUVIA_OPTIONAL_FIELD(phone, ruvia::String);
    RUVIA_OPTIONAL_FIELD(email, ruvia::String);
    RUVIA_OPTIONAL_FIELD_NAME("dept_id", deptId, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD(status, ruvia::String);
    RUVIA_OPTIONAL_FIELD(password, ruvia::String);
    RUVIA_OPTIONAL_FIELD_NAME("role_ids", roleIds, ruvia::Array<ruvia::Int64>);
    RUVIA_MODEL(UpdateUserBody, nickname, phone, email, deptId, status, password, roleIds);
};

struct UserOptionDto final {
    RUVIA_OPTIONAL_FIELD(id, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD(username, ruvia::String);
    RUVIA_OPTIONAL_FIELD(nickname, ruvia::String, RUVIA_OMIT_EMPTY);
    RUVIA_OPTIONAL_FIELD(phone, ruvia::String, RUVIA_OMIT_EMPTY);
    RUVIA_OPTIONAL_FIELD(email, ruvia::String, RUVIA_OMIT_EMPTY);
    RUVIA_MODEL(UserOptionDto, id, username, nickname, phone, email);
};

struct UserItemDto final {
    RUVIA_OPTIONAL_FIELD(id, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD(username, ruvia::String);
    RUVIA_OPTIONAL_FIELD(nickname, ruvia::String, RUVIA_OMIT_EMPTY);
    RUVIA_OPTIONAL_FIELD(phone, ruvia::String, RUVIA_OMIT_EMPTY);
    RUVIA_OPTIONAL_FIELD(email, ruvia::String, RUVIA_OMIT_EMPTY);
    RUVIA_OPTIONAL_FIELD_NAME("dept_id", deptId, ruvia::Int64, RUVIA_EMIT_NULL);
    RUVIA_OPTIONAL_FIELD_NAME("dept_name", deptName, ruvia::String, RUVIA_OMIT_EMPTY);
    RUVIA_OPTIONAL_FIELD(status, ruvia::String);
    RUVIA_OPTIONAL_FIELD(roles, ruvia::BoxedArray<role::RoleOptionDto>);
    RUVIA_MODEL(UserItemDto, id, username, nickname, phone, email, deptId, deptName, status, roles);
};

struct UserPageDataDto final {
    RUVIA_OPTIONAL_FIELD(list, ruvia::BoxedArray<UserItemDto>);
    RUVIA_OPTIONAL_FIELD(total, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD(page, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD_NAME("pageSize", pageSize, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD_NAME("totalPages", totalPages, ruvia::Int64);
    RUVIA_MODEL(UserPageDataDto, list, total, page, pageSize, totalPages);
};

struct UserPageResponse final {
    RUVIA_OPTIONAL_FIELD(code, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD(message, ruvia::String);
    RUVIA_OPTIONAL_FIELD(data, UserPageDataDto);
    RUVIA_MODEL(UserPageResponse, code, message, data);
};

struct UserDetailResponse final {
    RUVIA_OPTIONAL_FIELD(code, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD(message, ruvia::String);
    RUVIA_OPTIONAL_FIELD(data, UserItemDto);
    RUVIA_MODEL(UserDetailResponse, code, message, data);
};

struct UserOptionsResponse final {
    RUVIA_OPTIONAL_FIELD(code, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD(message, ruvia::String);
    RUVIA_OPTIONAL_FIELD(data, ruvia::BoxedArray<UserOptionDto>);
    RUVIA_MODEL(UserOptionsResponse, code, message, data);
};

} // namespace service::user
