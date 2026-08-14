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

RUVIA_REQUEST_MODEL(CreateUserBody, RUVIA_OPTIONAL_FIELD(username, ruvia::String),
                    RUVIA_OPTIONAL_FIELD(password, ruvia::String),
                    RUVIA_OPTIONAL_FIELD(nickname, ruvia::String),
                    RUVIA_OPTIONAL_FIELD(phone, ruvia::String),
                    RUVIA_OPTIONAL_FIELD(email, ruvia::String),
                    RUVIA_OPTIONAL_FIELD_NAME("dept_id", deptId, ruvia::Int64),
                    RUVIA_OPTIONAL_FIELD(status, ruvia::String),
                    RUVIA_OPTIONAL_FIELD_NAME("role_ids", roleIds, ruvia::Array<ruvia::Int64>));

RUVIA_REQUEST_MODEL(UpdateUserBody, RUVIA_OPTIONAL_FIELD(nickname, ruvia::String),
                    RUVIA_OPTIONAL_FIELD(phone, ruvia::String),
                    RUVIA_OPTIONAL_FIELD(email, ruvia::String),
                    RUVIA_OPTIONAL_FIELD_NAME("dept_id", deptId, ruvia::Int64),
                    RUVIA_OPTIONAL_FIELD(status, ruvia::String),
                    RUVIA_OPTIONAL_FIELD(password, ruvia::String),
                    RUVIA_OPTIONAL_FIELD_NAME("role_ids", roleIds, ruvia::Array<ruvia::Int64>));

RUVIA_RESPONSE_MODEL(UserOptionDto, RUVIA_OPTIONAL_FIELD(id, ruvia::Int64),
                     RUVIA_OPTIONAL_FIELD(username, ruvia::String),
                     RUVIA_OPTIONAL_FIELD(nickname, ruvia::String, RUVIA_OMIT_EMPTY),
                     RUVIA_OPTIONAL_FIELD(phone, ruvia::String, RUVIA_OMIT_EMPTY),
                     RUVIA_OPTIONAL_FIELD(email, ruvia::String, RUVIA_OMIT_EMPTY));

RUVIA_RESPONSE_MODEL(UserItemDto, RUVIA_OPTIONAL_FIELD(id, ruvia::Int64),
                     RUVIA_OPTIONAL_FIELD(username, ruvia::String),
                     RUVIA_OPTIONAL_FIELD(nickname, ruvia::String, RUVIA_OMIT_EMPTY),
                     RUVIA_OPTIONAL_FIELD(phone, ruvia::String, RUVIA_OMIT_EMPTY),
                     RUVIA_OPTIONAL_FIELD(email, ruvia::String, RUVIA_OMIT_EMPTY),
                     RUVIA_OPTIONAL_FIELD_NAME("dept_id", deptId, ruvia::Int64, RUVIA_EMIT_NULL),
                     RUVIA_OPTIONAL_FIELD_NAME("dept_name", deptName, ruvia::String,
                                               RUVIA_OMIT_EMPTY),
                     RUVIA_OPTIONAL_FIELD(status, ruvia::String),
                     RUVIA_OPTIONAL_FIELD(roles, ruvia::Array<role::RoleOptionDto>));

RUVIA_RESPONSE_MODEL(UserPageDataDto, RUVIA_OPTIONAL_FIELD(list, ruvia::Array<UserItemDto>),
                     RUVIA_OPTIONAL_FIELD(total, ruvia::Int64),
                     RUVIA_OPTIONAL_FIELD(page, ruvia::Int64),
                     RUVIA_OPTIONAL_FIELD_NAME("page_size", pageSize, ruvia::Int64),
                     RUVIA_OPTIONAL_FIELD_NAME("total_pages", totalPages, ruvia::Int64));

RUVIA_RESPONSE_MODEL(UserPageResponse, RUVIA_OPTIONAL_FIELD(code, ruvia::Int64),
                     RUVIA_OPTIONAL_FIELD(message, ruvia::String),
                     RUVIA_OPTIONAL_FIELD(data, UserPageDataDto));

RUVIA_RESPONSE_MODEL(UserDetailResponse, RUVIA_OPTIONAL_FIELD(code, ruvia::Int64),
                     RUVIA_OPTIONAL_FIELD(message, ruvia::String),
                     RUVIA_OPTIONAL_FIELD(data, UserItemDto));

RUVIA_RESPONSE_MODEL(UserOptionsResponse, RUVIA_OPTIONAL_FIELD(code, ruvia::Int64),
                     RUVIA_OPTIONAL_FIELD(message, ruvia::String),
                     RUVIA_OPTIONAL_FIELD(data, ruvia::Array<UserOptionDto>));

} // namespace service::user
