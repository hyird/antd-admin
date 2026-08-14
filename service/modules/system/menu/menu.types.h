#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include <ruvia/web/Model.h>

namespace service::menu {

RUVIA_REQUEST_MODEL(CreateMenuBody, RUVIA_OPTIONAL_FIELD(name, ruvia::String),
                    RUVIA_OPTIONAL_FIELD(path, ruvia::String),
                    RUVIA_OPTIONAL_FIELD(icon, ruvia::String),
                    RUVIA_OPTIONAL_FIELD(component, ruvia::String),
                    RUVIA_OPTIONAL_FIELD_NAME("parent_id", parentId, ruvia::Int64),
                    RUVIA_OPTIONAL_FIELD_NAME("sort_order", sortOrder, ruvia::Int64),
                    RUVIA_OPTIONAL_FIELD(type, ruvia::String),
                    RUVIA_OPTIONAL_FIELD(status, ruvia::String),
                    RUVIA_OPTIONAL_FIELD_NAME("permission_code", permissionCode, ruvia::String),
                    RUVIA_OPTIONAL_FIELD_NAME("is_default", isDefault, ruvia::Bool));

RUVIA_REQUEST_MODEL(UpdateMenuBody, RUVIA_OPTIONAL_FIELD(name, ruvia::String),
                    RUVIA_OPTIONAL_FIELD(path, ruvia::String),
                    RUVIA_OPTIONAL_FIELD(icon, ruvia::String),
                    RUVIA_OPTIONAL_FIELD(component, ruvia::String),
                    RUVIA_OPTIONAL_FIELD_NAME("parent_id", parentId, ruvia::Int64),
                    RUVIA_OPTIONAL_FIELD_NAME("sort_order", sortOrder, ruvia::Int64),
                    RUVIA_OPTIONAL_FIELD(type, ruvia::String),
                    RUVIA_OPTIONAL_FIELD(status, ruvia::String),
                    RUVIA_OPTIONAL_FIELD_NAME("permission_code", permissionCode, ruvia::String),
                    RUVIA_OPTIONAL_FIELD_NAME("is_default", isDefault, ruvia::Bool));

RUVIA_REQUEST_MODEL(ReorderMenuItemBody, RUVIA_OPTIONAL_FIELD(id, ruvia::Int64),
                    RUVIA_OPTIONAL_FIELD_NAME("sort_order", sortOrder, ruvia::Int64),
                    RUVIA_OPTIONAL_FIELD_NAME("parent_id", parentId, ruvia::Int64));

RUVIA_REQUEST_MODEL(ReorderMenuBody,
                    RUVIA_OPTIONAL_FIELD(items, ruvia::Array<ReorderMenuItemBody>));

RUVIA_REQUEST_MODEL(BatchCreateMenuButtonItemBody, RUVIA_OPTIONAL_FIELD(name, ruvia::String),
                    RUVIA_OPTIONAL_FIELD_NAME("permission_code", permissionCode, ruvia::String));

RUVIA_REQUEST_MODEL(BatchCreateMenuButtonsBody,
                    RUVIA_OPTIONAL_FIELD_NAME("parent_id", parentId, ruvia::Int64),
                    RUVIA_OPTIONAL_FIELD(items, ruvia::Array<BatchCreateMenuButtonItemBody>));

RUVIA_RESPONSE_MODEL(
    MenuDto, RUVIA_OPTIONAL_FIELD(id, ruvia::Int64), RUVIA_OPTIONAL_FIELD(name, ruvia::String),
    RUVIA_OPTIONAL_FIELD(path, ruvia::String, RUVIA_OMIT_EMPTY),
    RUVIA_OPTIONAL_FIELD(icon, ruvia::String, RUVIA_OMIT_EMPTY),
    RUVIA_OPTIONAL_FIELD_NAME("parent_id", parentId, ruvia::Int64, RUVIA_EMIT_NULL),
    RUVIA_OPTIONAL_FIELD_NAME("sort_order", sortOrder, ruvia::Int64),
    RUVIA_OPTIONAL_FIELD(type, ruvia::String),
    RUVIA_OPTIONAL_FIELD(component, ruvia::String, RUVIA_OMIT_EMPTY),
    RUVIA_OPTIONAL_FIELD(status, ruvia::String),
    RUVIA_OPTIONAL_FIELD_NAME("permission_code", permissionCode, ruvia::String, RUVIA_OMIT_EMPTY),
    RUVIA_OPTIONAL_FIELD_NAME("is_default", isDefault, ruvia::Bool),
    RUVIA_OPTIONAL_FIELD_NAME("full_path", fullPath, ruvia::String, RUVIA_OMIT_EMPTY),
    RUVIA_OPTIONAL_FIELD(children, ruvia::BoxedArray<MenuDto>, RUVIA_OMIT_EMPTY));

RUVIA_RESPONSE_MODEL(MenuListResponse, RUVIA_OPTIONAL_FIELD(code, ruvia::Int64),
                     RUVIA_OPTIONAL_FIELD(message, ruvia::String),
                     RUVIA_OPTIONAL_FIELD(data, ruvia::BoxedArray<MenuDto>));

RUVIA_RESPONSE_MODEL(MenuPageDataDto, RUVIA_OPTIONAL_FIELD(list, ruvia::BoxedArray<MenuDto>),
                     RUVIA_OPTIONAL_FIELD(total, ruvia::Int64),
                     RUVIA_OPTIONAL_FIELD(page, ruvia::Int64),
                     RUVIA_OPTIONAL_FIELD_NAME("page_size", pageSize, ruvia::Int64),
                     RUVIA_OPTIONAL_FIELD_NAME("total_pages", totalPages, ruvia::Int64));

RUVIA_RESPONSE_MODEL(MenuPageResponse, RUVIA_OPTIONAL_FIELD(code, ruvia::Int64),
                     RUVIA_OPTIONAL_FIELD(message, ruvia::String),
                     RUVIA_OPTIONAL_FIELD(data, MenuPageDataDto));

RUVIA_RESPONSE_MODEL(MenuDetailResponse, RUVIA_OPTIONAL_FIELD(code, ruvia::Int64),
                     RUVIA_OPTIONAL_FIELD(message, ruvia::String),
                     RUVIA_OPTIONAL_FIELD(data, MenuDto));

} // namespace service::menu
