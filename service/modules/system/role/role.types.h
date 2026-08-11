#pragma once

#include <optional>
#include <string>

#include <ruvia/web/Model.h>

namespace service::role {

struct CreateRoleBody final {
    RUVIA_OPTIONAL_FIELD(code, ruvia::String);
    RUVIA_OPTIONAL_FIELD(name, ruvia::String);
    RUVIA_OPTIONAL_FIELD(status, ruvia::String);
    RUVIA_OPTIONAL_FIELD_NAME("menu_ids", menuIds, ruvia::Array<ruvia::Int64>);
    RUVIA_MODEL(CreateRoleBody, code, name, status, menuIds);
};

struct UpdateRoleBody final {
    RUVIA_OPTIONAL_FIELD(code, ruvia::String);
    RUVIA_OPTIONAL_FIELD(name, ruvia::String);
    RUVIA_OPTIONAL_FIELD(status, ruvia::String);
    RUVIA_OPTIONAL_FIELD_NAME("menu_ids", menuIds, ruvia::Array<ruvia::Int64>);
    RUVIA_MODEL(UpdateRoleBody, code, name, status, menuIds);
};

struct RoleItemDto final {
    RUVIA_OPTIONAL_FIELD(id, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD(name, ruvia::String);
    RUVIA_OPTIONAL_FIELD(code, ruvia::String);
    RUVIA_OPTIONAL_FIELD(status, ruvia::String);
    RUVIA_OPTIONAL_FIELD_NAME("menu_ids", menuIds, ruvia::Array<ruvia::Int64>, RUVIA_OMIT_EMPTY);
    RUVIA_MODEL(RoleItemDto, id, name, code, status, menuIds);
};

struct RoleMenuDto final {
    RUVIA_OPTIONAL_FIELD(id, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD(name, ruvia::String);
    RUVIA_OPTIONAL_FIELD(type, ruvia::String);
    RUVIA_OPTIONAL_FIELD_NAME("parent_id", parentId, ruvia::Int64, RUVIA_EMIT_NULL);
    RUVIA_MODEL(RoleMenuDto, id, name, type, parentId);
};

struct RoleDetailDto final {
    RUVIA_OPTIONAL_FIELD(id, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD(name, ruvia::String);
    RUVIA_OPTIONAL_FIELD(code, ruvia::String);
    RUVIA_OPTIONAL_FIELD(status, ruvia::String);
    RUVIA_OPTIONAL_FIELD_NAME("menu_ids", menuIds, ruvia::Array<ruvia::Int64>);
    RUVIA_OPTIONAL_FIELD(menus, ruvia::Array<RoleMenuDto>);
    RUVIA_MODEL(RoleDetailDto, id, name, code, status, menuIds, menus);
};

struct RoleOptionDto final {
    RUVIA_OPTIONAL_FIELD(id, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD(name, ruvia::String);
    RUVIA_OPTIONAL_FIELD(code, ruvia::String);
    RUVIA_MODEL(RoleOptionDto, id, name, code);
};

struct RolePageDataDto final {
    RUVIA_OPTIONAL_FIELD(list, ruvia::BoxedArray<RoleItemDto>);
    RUVIA_OPTIONAL_FIELD(total, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD(page, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD_NAME("pageSize", pageSize, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD_NAME("totalPages", totalPages, ruvia::Int64);
    RUVIA_MODEL(RolePageDataDto, list, total, page, pageSize, totalPages);
};

struct RolePageResponse final {
    RUVIA_OPTIONAL_FIELD(code, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD(message, ruvia::String);
    RUVIA_OPTIONAL_FIELD(data, RolePageDataDto);
    RUVIA_MODEL(RolePageResponse, code, message, data);
};

struct RoleDetailResponse final {
    RUVIA_OPTIONAL_FIELD(code, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD(message, ruvia::String);
    RUVIA_OPTIONAL_FIELD(data, RoleDetailDto);
    RUVIA_MODEL(RoleDetailResponse, code, message, data);
};

struct RoleOptionsResponse final {
    RUVIA_OPTIONAL_FIELD(code, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD(message, ruvia::String);
    RUVIA_OPTIONAL_FIELD(data, ruvia::BoxedArray<RoleOptionDto>);
    RUVIA_MODEL(RoleOptionsResponse, code, message, data);
};

} // namespace service::role
