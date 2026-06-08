#pragma once

#include <optional>
#include <string>

#include <cyra/http/Model.h>

namespace service::role {

CYRA_MODEL(CreateRoleBody, CYRA_FIELD(code, cyra::String), CYRA_FIELD(name, cyra::String),
           CYRA_FIELD(status, cyra::String),
           CYRA_FIELD_NAME("menu_ids", menuIds, cyra::Array<cyra::Int64>));

CYRA_MODEL(UpdateRoleBody, CYRA_FIELD(code, cyra::String), CYRA_FIELD(name, cyra::String),
           CYRA_FIELD(status, cyra::String),
           CYRA_FIELD_NAME("menu_ids", menuIds, cyra::Array<cyra::Int64>));

CYRA_MODEL(RoleItemDto, CYRA_FIELD(id, cyra::Int64), CYRA_FIELD(name, cyra::String),
           CYRA_FIELD(code, cyra::String), CYRA_FIELD(status, cyra::String),
           CYRA_FIELD_NAME("menu_ids", menuIds, cyra::Array<cyra::Int64>, CYRA_OMIT_EMPTY));

CYRA_MODEL(RoleMenuDto, CYRA_FIELD(id, cyra::Int64), CYRA_FIELD(name, cyra::String),
           CYRA_FIELD(type, cyra::String),
           CYRA_FIELD_NAME("parent_id", parentId, cyra::Int64, CYRA_EMIT_NULL));

CYRA_MODEL(RoleDetailDto, CYRA_FIELD(id, cyra::Int64), CYRA_FIELD(name, cyra::String),
           CYRA_FIELD(code, cyra::String), CYRA_FIELD(status, cyra::String),
           CYRA_FIELD_NAME("menu_ids", menuIds, cyra::Array<cyra::Int64>),
           CYRA_FIELD(menus, cyra::Array<RoleMenuDto>));

CYRA_MODEL(RoleOptionDto, CYRA_FIELD(id, cyra::Int64), CYRA_FIELD(name, cyra::String),
           CYRA_FIELD(code, cyra::String));

CYRA_MODEL(RolePageDataDto, CYRA_FIELD(list, cyra::List<RoleItemDto>),
           CYRA_FIELD(total, cyra::Int64), CYRA_FIELD(page, cyra::Int64),
           CYRA_FIELD_NAME("pageSize", pageSize, cyra::Int64),
           CYRA_FIELD_NAME("totalPages", totalPages, cyra::Int64));

CYRA_MODEL(RolePageResponse, CYRA_FIELD(code, cyra::Int64), CYRA_FIELD(message, cyra::String),
           CYRA_FIELD(data, RolePageDataDto));

CYRA_MODEL(RoleDetailResponse, CYRA_FIELD(code, cyra::Int64), CYRA_FIELD(message, cyra::String),
           CYRA_FIELD(data, RoleDetailDto));

CYRA_MODEL(RoleOptionsResponse, CYRA_FIELD(code, cyra::Int64), CYRA_FIELD(message, cyra::String),
           CYRA_FIELD(data, cyra::List<RoleOptionDto>));

} // namespace service::role
