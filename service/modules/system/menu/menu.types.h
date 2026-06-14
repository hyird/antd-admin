#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include <ruvia/http/Model.h>

namespace service::menu {

RUVIA_MODEL(CreateMenuBody, RUVIA_FIELD(name, ruvia::String), RUVIA_FIELD(path, ruvia::String),
           RUVIA_FIELD(icon, ruvia::String), RUVIA_FIELD(component, ruvia::String),
           RUVIA_FIELD_NAME("parent_id", parentId, ruvia::Int64),
           RUVIA_FIELD_NAME("sort_order", sortOrder, ruvia::Int64), RUVIA_FIELD(type, ruvia::String),
           RUVIA_FIELD(status, ruvia::String),
           RUVIA_FIELD_NAME("permission_code", permissionCode, ruvia::String),
           RUVIA_FIELD_NAME("is_default", isDefault, ruvia::Bool));

RUVIA_MODEL(UpdateMenuBody, RUVIA_FIELD(name, ruvia::String), RUVIA_FIELD(path, ruvia::String),
           RUVIA_FIELD(icon, ruvia::String), RUVIA_FIELD(component, ruvia::String),
           RUVIA_FIELD_NAME("parent_id", parentId, ruvia::Int64),
           RUVIA_FIELD_NAME("sort_order", sortOrder, ruvia::Int64), RUVIA_FIELD(type, ruvia::String),
           RUVIA_FIELD(status, ruvia::String),
           RUVIA_FIELD_NAME("permission_code", permissionCode, ruvia::String),
           RUVIA_FIELD_NAME("is_default", isDefault, ruvia::Bool));

RUVIA_MODEL(ReorderMenuItemBody, RUVIA_FIELD(id, ruvia::Int64),
           RUVIA_FIELD_NAME("sort_order", sortOrder, ruvia::Int64),
           RUVIA_FIELD_NAME("parent_id", parentId, ruvia::Int64));

RUVIA_MODEL(ReorderMenuBody, RUVIA_FIELD(items, ruvia::Array<ReorderMenuItemBody>));

RUVIA_MODEL(BatchCreateMenuButtonItemBody, RUVIA_FIELD(name, ruvia::String),
           RUVIA_FIELD_NAME("permission_code", permissionCode, ruvia::String));

RUVIA_MODEL(BatchCreateMenuButtonsBody, RUVIA_FIELD_NAME("parent_id", parentId, ruvia::Int64),
           RUVIA_FIELD(items, ruvia::Array<BatchCreateMenuButtonItemBody>));

RUVIA_MODEL(MenuDto, RUVIA_FIELD(id, ruvia::Int64), RUVIA_FIELD(name, ruvia::String),
           RUVIA_FIELD(path, ruvia::String, RUVIA_OMIT_EMPTY),
           RUVIA_FIELD(icon, ruvia::String, RUVIA_OMIT_EMPTY),
           RUVIA_FIELD_NAME("parent_id", parentId, ruvia::Int64, RUVIA_EMIT_NULL),
           RUVIA_FIELD_NAME("sort_order", sortOrder, ruvia::Int64), RUVIA_FIELD(type, ruvia::String),
           RUVIA_FIELD(component, ruvia::String, RUVIA_OMIT_EMPTY), RUVIA_FIELD(status, ruvia::String),
           RUVIA_FIELD_NAME("permission_code", permissionCode, ruvia::String, RUVIA_OMIT_EMPTY),
           RUVIA_FIELD_NAME("is_default", isDefault, ruvia::Bool),
           RUVIA_FIELD_NAME("full_path", fullPath, ruvia::String, RUVIA_OMIT_EMPTY),
           RUVIA_FIELD(children, ruvia::List<MenuDto>, RUVIA_OMIT_EMPTY));

RUVIA_MODEL(MenuListResponse, RUVIA_FIELD(code, ruvia::Int64), RUVIA_FIELD(message, ruvia::String),
           RUVIA_FIELD(data, ruvia::List<MenuDto>));

RUVIA_MODEL(MenuPageDataDto, RUVIA_FIELD(list, ruvia::List<MenuDto>), RUVIA_FIELD(total, ruvia::Int64),
           RUVIA_FIELD(page, ruvia::Int64), RUVIA_FIELD_NAME("pageSize", pageSize, ruvia::Int64),
           RUVIA_FIELD_NAME("totalPages", totalPages, ruvia::Int64));

RUVIA_MODEL(MenuPageResponse, RUVIA_FIELD(code, ruvia::Int64), RUVIA_FIELD(message, ruvia::String),
           RUVIA_FIELD(data, MenuPageDataDto));

RUVIA_MODEL(MenuDetailResponse, RUVIA_FIELD(code, ruvia::Int64), RUVIA_FIELD(message, ruvia::String),
           RUVIA_FIELD(data, MenuDto));

} // namespace service::menu
