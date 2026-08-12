#pragma once

#include <optional>
#include <string>

#include <ruvia/web/Model.h>

namespace service::role {

RUVIA_REQUEST_MODEL(CreateRoleBody, RUVIA_OPTIONAL_FIELD(code, ruvia::String),
                    RUVIA_OPTIONAL_FIELD(name, ruvia::String),
                    RUVIA_OPTIONAL_FIELD(status, ruvia::String),
                    RUVIA_OPTIONAL_FIELD_NAME("menu_ids", menuIds, ruvia::Array<ruvia::Int64>));

RUVIA_REQUEST_MODEL(UpdateRoleBody, RUVIA_OPTIONAL_FIELD(code, ruvia::String),
                    RUVIA_OPTIONAL_FIELD(name, ruvia::String),
                    RUVIA_OPTIONAL_FIELD(status, ruvia::String),
                    RUVIA_OPTIONAL_FIELD_NAME("menu_ids", menuIds, ruvia::Array<ruvia::Int64>));

RUVIA_RESPONSE_MODEL(
    RoleItemDto, RUVIA_OPTIONAL_FIELD(id, ruvia::Int64), RUVIA_OPTIONAL_FIELD(name, ruvia::String),
    RUVIA_OPTIONAL_FIELD(code, ruvia::String), RUVIA_OPTIONAL_FIELD(status, ruvia::String),
    RUVIA_OPTIONAL_FIELD_NAME("menu_ids", menuIds, ruvia::Array<ruvia::Int64>, RUVIA_OMIT_EMPTY));

RUVIA_RESPONSE_MODEL(RoleMenuDto, RUVIA_OPTIONAL_FIELD(id, ruvia::Int64),
                     RUVIA_OPTIONAL_FIELD(name, ruvia::String),
                     RUVIA_OPTIONAL_FIELD(type, ruvia::String),
                     RUVIA_OPTIONAL_FIELD_NAME("parent_id", parentId, ruvia::Int64,
                                               RUVIA_EMIT_NULL));

RUVIA_RESPONSE_MODEL(RoleDetailDto, RUVIA_OPTIONAL_FIELD(id, ruvia::Int64),
                     RUVIA_OPTIONAL_FIELD(name, ruvia::String),
                     RUVIA_OPTIONAL_FIELD(code, ruvia::String),
                     RUVIA_OPTIONAL_FIELD(status, ruvia::String),
                     RUVIA_OPTIONAL_FIELD_NAME("menu_ids", menuIds, ruvia::Array<ruvia::Int64>),
                     RUVIA_OPTIONAL_FIELD(menus, ruvia::Array<RoleMenuDto>));

RUVIA_RESPONSE_MODEL(RoleOptionDto, RUVIA_OPTIONAL_FIELD(id, ruvia::Int64),
                     RUVIA_OPTIONAL_FIELD(name, ruvia::String),
                     RUVIA_OPTIONAL_FIELD(code, ruvia::String));

RUVIA_RESPONSE_MODEL(RolePageDataDto, RUVIA_OPTIONAL_FIELD(list, ruvia::BoxedArray<RoleItemDto>),
                     RUVIA_OPTIONAL_FIELD(total, ruvia::Int64),
                     RUVIA_OPTIONAL_FIELD(page, ruvia::Int64),
                     RUVIA_OPTIONAL_FIELD_NAME("pageSize", pageSize, ruvia::Int64),
                     RUVIA_OPTIONAL_FIELD_NAME("totalPages", totalPages, ruvia::Int64));

RUVIA_RESPONSE_MODEL(RolePageResponse, RUVIA_OPTIONAL_FIELD(code, ruvia::Int64),
                     RUVIA_OPTIONAL_FIELD(message, ruvia::String),
                     RUVIA_OPTIONAL_FIELD(data, RolePageDataDto));

RUVIA_RESPONSE_MODEL(RoleDetailResponse, RUVIA_OPTIONAL_FIELD(code, ruvia::Int64),
                     RUVIA_OPTIONAL_FIELD(message, ruvia::String),
                     RUVIA_OPTIONAL_FIELD(data, RoleDetailDto));

RUVIA_RESPONSE_MODEL(RoleOptionsResponse, RUVIA_OPTIONAL_FIELD(code, ruvia::Int64),
                     RUVIA_OPTIONAL_FIELD(message, ruvia::String),
                     RUVIA_OPTIONAL_FIELD(data, ruvia::BoxedArray<RoleOptionDto>));

} // namespace service::role
