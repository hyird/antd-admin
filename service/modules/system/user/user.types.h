#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

#include <ruvia/http/Model.h>

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

RUVIA_MODEL(CreateUserBody, RUVIA_FIELD(username, ruvia::String), RUVIA_FIELD(password, ruvia::String),
           RUVIA_FIELD(nickname, ruvia::String), RUVIA_FIELD(phone, ruvia::String),
           RUVIA_FIELD(email, ruvia::String), RUVIA_FIELD_NAME("dept_id", deptId, ruvia::Int64),
           RUVIA_FIELD(status, ruvia::String),
           RUVIA_FIELD_NAME("role_ids", roleIds, ruvia::Array<ruvia::Int64>));

RUVIA_MODEL(UpdateUserBody, RUVIA_FIELD(nickname, ruvia::String), RUVIA_FIELD(phone, ruvia::String),
           RUVIA_FIELD(email, ruvia::String), RUVIA_FIELD_NAME("dept_id", deptId, ruvia::Int64),
           RUVIA_FIELD(status, ruvia::String), RUVIA_FIELD(password, ruvia::String),
           RUVIA_FIELD_NAME("role_ids", roleIds, ruvia::Array<ruvia::Int64>));

RUVIA_MODEL(UserOptionDto, RUVIA_FIELD(id, ruvia::Int64), RUVIA_FIELD(username, ruvia::String),
           RUVIA_FIELD(nickname, ruvia::String, RUVIA_OMIT_EMPTY),
           RUVIA_FIELD(phone, ruvia::String, RUVIA_OMIT_EMPTY),
           RUVIA_FIELD(email, ruvia::String, RUVIA_OMIT_EMPTY));

RUVIA_MODEL(UserItemDto, RUVIA_FIELD(id, ruvia::Int64), RUVIA_FIELD(username, ruvia::String),
           RUVIA_FIELD(nickname, ruvia::String, RUVIA_OMIT_EMPTY),
           RUVIA_FIELD(phone, ruvia::String, RUVIA_OMIT_EMPTY),
           RUVIA_FIELD(email, ruvia::String, RUVIA_OMIT_EMPTY),
           RUVIA_FIELD_NAME("dept_id", deptId, ruvia::Int64, RUVIA_EMIT_NULL),
           RUVIA_FIELD_NAME("dept_name", deptName, ruvia::String, RUVIA_OMIT_EMPTY),
           RUVIA_FIELD(status, ruvia::String), RUVIA_FIELD(roles, ruvia::List<role::RoleOptionDto>));

RUVIA_MODEL(UserPageDataDto, RUVIA_FIELD(list, ruvia::List<UserItemDto>),
           RUVIA_FIELD(total, ruvia::Int64), RUVIA_FIELD(page, ruvia::Int64),
           RUVIA_FIELD_NAME("pageSize", pageSize, ruvia::Int64),
           RUVIA_FIELD_NAME("totalPages", totalPages, ruvia::Int64));

RUVIA_MODEL(UserPageResponse, RUVIA_FIELD(code, ruvia::Int64), RUVIA_FIELD(message, ruvia::String),
           RUVIA_FIELD(data, UserPageDataDto));

RUVIA_MODEL(UserDetailResponse, RUVIA_FIELD(code, ruvia::Int64), RUVIA_FIELD(message, ruvia::String),
           RUVIA_FIELD(data, UserItemDto));

RUVIA_MODEL(UserOptionsResponse, RUVIA_FIELD(code, ruvia::Int64), RUVIA_FIELD(message, ruvia::String),
           RUVIA_FIELD(data, ruvia::List<UserOptionDto>));

} // namespace service::user
