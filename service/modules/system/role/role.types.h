#pragma once

#include <optional>
#include <string>

#include <ruvia/http/Model.h>

namespace service::role {

RUVIA_MODEL(CreateRoleBody, RUVIA_FIELD(code, ruvia::String), RUVIA_FIELD(name, ruvia::String),
           RUVIA_FIELD(status, ruvia::String),
           RUVIA_FIELD_NAME("menu_ids", menuIds, ruvia::Array<ruvia::Int64>));

RUVIA_MODEL(UpdateRoleBody, RUVIA_FIELD(code, ruvia::String), RUVIA_FIELD(name, ruvia::String),
           RUVIA_FIELD(status, ruvia::String),
           RUVIA_FIELD_NAME("menu_ids", menuIds, ruvia::Array<ruvia::Int64>));

RUVIA_MODEL(RoleItemDto, RUVIA_FIELD(id, ruvia::Int64), RUVIA_FIELD(name, ruvia::String),
           RUVIA_FIELD(code, ruvia::String), RUVIA_FIELD(status, ruvia::String),
           RUVIA_FIELD_NAME("menu_ids", menuIds, ruvia::Array<ruvia::Int64>, RUVIA_OMIT_EMPTY));

RUVIA_MODEL(RoleMenuDto, RUVIA_FIELD(id, ruvia::Int64), RUVIA_FIELD(name, ruvia::String),
           RUVIA_FIELD(type, ruvia::String),
           RUVIA_FIELD_NAME("parent_id", parentId, ruvia::Int64, RUVIA_EMIT_NULL));

RUVIA_MODEL(RoleDetailDto, RUVIA_FIELD(id, ruvia::Int64), RUVIA_FIELD(name, ruvia::String),
           RUVIA_FIELD(code, ruvia::String), RUVIA_FIELD(status, ruvia::String),
           RUVIA_FIELD_NAME("menu_ids", menuIds, ruvia::Array<ruvia::Int64>),
           RUVIA_FIELD(menus, ruvia::Array<RoleMenuDto>));

RUVIA_MODEL(RoleOptionDto, RUVIA_FIELD(id, ruvia::Int64), RUVIA_FIELD(name, ruvia::String),
           RUVIA_FIELD(code, ruvia::String));

RUVIA_MODEL(RolePageDataDto, RUVIA_FIELD(list, ruvia::List<RoleItemDto>),
           RUVIA_FIELD(total, ruvia::Int64), RUVIA_FIELD(page, ruvia::Int64),
           RUVIA_FIELD_NAME("pageSize", pageSize, ruvia::Int64),
           RUVIA_FIELD_NAME("totalPages", totalPages, ruvia::Int64));

RUVIA_MODEL(RolePageResponse, RUVIA_FIELD(code, ruvia::Int64), RUVIA_FIELD(message, ruvia::String),
           RUVIA_FIELD(data, RolePageDataDto));

RUVIA_MODEL(RoleDetailResponse, RUVIA_FIELD(code, ruvia::Int64), RUVIA_FIELD(message, ruvia::String),
           RUVIA_FIELD(data, RoleDetailDto));

RUVIA_MODEL(RoleOptionsResponse, RUVIA_FIELD(code, ruvia::Int64), RUVIA_FIELD(message, ruvia::String),
           RUVIA_FIELD(data, ruvia::List<RoleOptionDto>));

} // namespace service::role
