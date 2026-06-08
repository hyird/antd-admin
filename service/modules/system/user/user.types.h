#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

#include <cyra/http/Model.h>

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

CYRA_MODEL(CreateUserBody, CYRA_FIELD(username, cyra::String), CYRA_FIELD(password, cyra::String),
           CYRA_FIELD(nickname, cyra::String), CYRA_FIELD(phone, cyra::String),
           CYRA_FIELD(email, cyra::String), CYRA_FIELD_NAME("dept_id", deptId, cyra::Int64),
           CYRA_FIELD(status, cyra::String),
           CYRA_FIELD_NAME("role_ids", roleIds, cyra::Array<cyra::Int64>));

CYRA_MODEL(UpdateUserBody, CYRA_FIELD(nickname, cyra::String), CYRA_FIELD(phone, cyra::String),
           CYRA_FIELD(email, cyra::String), CYRA_FIELD_NAME("dept_id", deptId, cyra::Int64),
           CYRA_FIELD(status, cyra::String), CYRA_FIELD(password, cyra::String),
           CYRA_FIELD_NAME("role_ids", roleIds, cyra::Array<cyra::Int64>));

CYRA_MODEL(UserOptionDto, CYRA_FIELD(id, cyra::Int64), CYRA_FIELD(username, cyra::String),
           CYRA_FIELD(nickname, cyra::String, CYRA_OMIT_EMPTY),
           CYRA_FIELD(phone, cyra::String, CYRA_OMIT_EMPTY),
           CYRA_FIELD(email, cyra::String, CYRA_OMIT_EMPTY));

CYRA_MODEL(UserItemDto, CYRA_FIELD(id, cyra::Int64), CYRA_FIELD(username, cyra::String),
           CYRA_FIELD(nickname, cyra::String, CYRA_OMIT_EMPTY),
           CYRA_FIELD(phone, cyra::String, CYRA_OMIT_EMPTY),
           CYRA_FIELD(email, cyra::String, CYRA_OMIT_EMPTY),
           CYRA_FIELD_NAME("dept_id", deptId, cyra::Int64, CYRA_EMIT_NULL),
           CYRA_FIELD_NAME("dept_name", deptName, cyra::String, CYRA_OMIT_EMPTY),
           CYRA_FIELD(status, cyra::String), CYRA_FIELD(roles, cyra::List<role::RoleOptionDto>));

CYRA_MODEL(UserPageDataDto, CYRA_FIELD(list, cyra::List<UserItemDto>),
           CYRA_FIELD(total, cyra::Int64), CYRA_FIELD(page, cyra::Int64),
           CYRA_FIELD_NAME("pageSize", pageSize, cyra::Int64),
           CYRA_FIELD_NAME("totalPages", totalPages, cyra::Int64));

CYRA_MODEL(UserPageResponse, CYRA_FIELD(code, cyra::Int64), CYRA_FIELD(message, cyra::String),
           CYRA_FIELD(data, UserPageDataDto));

CYRA_MODEL(UserDetailResponse, CYRA_FIELD(code, cyra::Int64), CYRA_FIELD(message, cyra::String),
           CYRA_FIELD(data, UserItemDto));

CYRA_MODEL(UserOptionsResponse, CYRA_FIELD(code, cyra::Int64), CYRA_FIELD(message, cyra::String),
           CYRA_FIELD(data, cyra::List<UserOptionDto>));

} // namespace service::user
