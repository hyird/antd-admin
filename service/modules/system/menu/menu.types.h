#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include <cyra/http/Model.h>

namespace service::menu {

CYRA_MODEL(CreateMenuBody, CYRA_FIELD(name, cyra::String), CYRA_FIELD(path, cyra::String),
           CYRA_FIELD(icon, cyra::String), CYRA_FIELD(component, cyra::String),
           CYRA_FIELD_NAME("parent_id", parentId, cyra::Int64),
           CYRA_FIELD_NAME("sort_order", sortOrder, cyra::Int64), CYRA_FIELD(type, cyra::String),
           CYRA_FIELD(status, cyra::String),
           CYRA_FIELD_NAME("permission_code", permissionCode, cyra::String),
           CYRA_FIELD_NAME("is_default", isDefault, cyra::Bool));

CYRA_MODEL(UpdateMenuBody, CYRA_FIELD(name, cyra::String), CYRA_FIELD(path, cyra::String),
           CYRA_FIELD(icon, cyra::String), CYRA_FIELD(component, cyra::String),
           CYRA_FIELD_NAME("parent_id", parentId, cyra::Int64),
           CYRA_FIELD_NAME("sort_order", sortOrder, cyra::Int64), CYRA_FIELD(type, cyra::String),
           CYRA_FIELD(status, cyra::String),
           CYRA_FIELD_NAME("permission_code", permissionCode, cyra::String),
           CYRA_FIELD_NAME("is_default", isDefault, cyra::Bool));

CYRA_MODEL(ReorderMenuItemBody, CYRA_FIELD(id, cyra::Int64),
           CYRA_FIELD_NAME("sort_order", sortOrder, cyra::Int64),
           CYRA_FIELD_NAME("parent_id", parentId, cyra::Int64));

CYRA_MODEL(ReorderMenuBody, CYRA_FIELD(items, cyra::Array<ReorderMenuItemBody>));

CYRA_MODEL(BatchCreateMenuButtonItemBody, CYRA_FIELD(name, cyra::String),
           CYRA_FIELD_NAME("permission_code", permissionCode, cyra::String));

CYRA_MODEL(BatchCreateMenuButtonsBody, CYRA_FIELD_NAME("parent_id", parentId, cyra::Int64),
           CYRA_FIELD(items, cyra::Array<BatchCreateMenuButtonItemBody>));

CYRA_MODEL(MenuDto, CYRA_FIELD(id, cyra::Int64), CYRA_FIELD(name, cyra::String),
           CYRA_FIELD(path, cyra::String, CYRA_OMIT_EMPTY),
           CYRA_FIELD(icon, cyra::String, CYRA_OMIT_EMPTY),
           CYRA_FIELD_NAME("parent_id", parentId, cyra::Int64, CYRA_EMIT_NULL),
           CYRA_FIELD_NAME("sort_order", sortOrder, cyra::Int64), CYRA_FIELD(type, cyra::String),
           CYRA_FIELD(component, cyra::String, CYRA_OMIT_EMPTY), CYRA_FIELD(status, cyra::String),
           CYRA_FIELD_NAME("permission_code", permissionCode, cyra::String, CYRA_OMIT_EMPTY),
           CYRA_FIELD_NAME("is_default", isDefault, cyra::Bool),
           CYRA_FIELD_NAME("full_path", fullPath, cyra::String, CYRA_OMIT_EMPTY),
           CYRA_FIELD(children, cyra::List<MenuDto>, CYRA_OMIT_EMPTY));

CYRA_MODEL(MenuListResponse, CYRA_FIELD(code, cyra::Int64), CYRA_FIELD(message, cyra::String),
           CYRA_FIELD(data, cyra::List<MenuDto>));

CYRA_MODEL(MenuPageDataDto, CYRA_FIELD(list, cyra::List<MenuDto>), CYRA_FIELD(total, cyra::Int64),
           CYRA_FIELD(page, cyra::Int64), CYRA_FIELD_NAME("pageSize", pageSize, cyra::Int64),
           CYRA_FIELD_NAME("totalPages", totalPages, cyra::Int64));

CYRA_MODEL(MenuPageResponse, CYRA_FIELD(code, cyra::Int64), CYRA_FIELD(message, cyra::String),
           CYRA_FIELD(data, MenuPageDataDto));

CYRA_MODEL(MenuDetailResponse, CYRA_FIELD(code, cyra::Int64), CYRA_FIELD(message, cyra::String),
           CYRA_FIELD(data, MenuDto));

} // namespace service::menu
