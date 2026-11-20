#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include <model.h>

#include "service/modules/system/role/role.types.h"

namespace service::user {

RUVIA_MODEL(UserOptionDto, RUVIA_OPTIONAL_FIELD(id, ruvia::int64),
            RUVIA_OPTIONAL_FIELD(username, ruvia::string),
            RUVIA_OPTIONAL_FIELD(nickname, ruvia::string, RUVIA_OMIT_EMPTY),
            RUVIA_OPTIONAL_FIELD(phone, ruvia::string, RUVIA_OMIT_EMPTY),
            RUVIA_OPTIONAL_FIELD(email, ruvia::string, RUVIA_OMIT_EMPTY));

RUVIA_MODEL(UserItemDto, RUVIA_OPTIONAL_FIELD(id, ruvia::int64),
            RUVIA_OPTIONAL_FIELD(username, ruvia::string),
            RUVIA_OPTIONAL_FIELD(nickname, ruvia::string, RUVIA_OMIT_EMPTY),
            RUVIA_OPTIONAL_FIELD(phone, ruvia::string, RUVIA_OMIT_EMPTY),
            RUVIA_OPTIONAL_FIELD(email, ruvia::string, RUVIA_OMIT_EMPTY),
            RUVIA_OPTIONAL_FIELD_NAME("dept_id", deptId, ruvia::int64, RUVIA_EMIT_NULL),
            RUVIA_OPTIONAL_FIELD_NAME("dept_name", deptName, ruvia::string, RUVIA_OMIT_EMPTY),
            RUVIA_OPTIONAL_FIELD(status, ruvia::string),
            RUVIA_OPTIONAL_FIELD(roles, ruvia::array<role::RoleOptionDto>));

RUVIA_MODEL(UserPageDataDto, RUVIA_OPTIONAL_FIELD(list, ruvia::array<UserItemDto>),
            RUVIA_OPTIONAL_FIELD(total, ruvia::int64), RUVIA_OPTIONAL_FIELD(page, ruvia::int64),
            RUVIA_OPTIONAL_FIELD_NAME("page_size", pageSize, ruvia::int64),
            RUVIA_OPTIONAL_FIELD_NAME("total_pages", totalPages, ruvia::int64));

RUVIA_MODEL(UserPageResponse, RUVIA_OPTIONAL_FIELD(code, ruvia::int64),
            RUVIA_OPTIONAL_FIELD(message, ruvia::string),
            RUVIA_OPTIONAL_FIELD(data, UserPageDataDto));

RUVIA_MODEL(UserDetailResponse, RUVIA_OPTIONAL_FIELD(code, ruvia::int64),
            RUVIA_OPTIONAL_FIELD(message, ruvia::string), RUVIA_OPTIONAL_FIELD(data, UserItemDto));

RUVIA_MODEL(UserOptionsResponse, RUVIA_OPTIONAL_FIELD(code, ruvia::int64),
            RUVIA_OPTIONAL_FIELD(message, ruvia::string),
            RUVIA_OPTIONAL_FIELD(data, ruvia::array<UserOptionDto>));

} // namespace service::user
