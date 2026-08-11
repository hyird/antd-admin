#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include <ruvia/web/Model.h>

namespace service::menu {

struct CreateMenuBody final {
    RUVIA_OPTIONAL_FIELD(name, ruvia::String);
    RUVIA_OPTIONAL_FIELD(path, ruvia::String);
    RUVIA_OPTIONAL_FIELD(icon, ruvia::String);
    RUVIA_OPTIONAL_FIELD(component, ruvia::String);
    RUVIA_OPTIONAL_FIELD_NAME("parent_id", parentId, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD_NAME("sort_order", sortOrder, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD(type, ruvia::String);
    RUVIA_OPTIONAL_FIELD(status, ruvia::String);
    RUVIA_OPTIONAL_FIELD_NAME("permission_code", permissionCode, ruvia::String);
    RUVIA_OPTIONAL_FIELD_NAME("is_default", isDefault, ruvia::Bool);
    RUVIA_MODEL(CreateMenuBody, name, path, icon, component, parentId, sortOrder, type, status,
                permissionCode, isDefault);
};

struct UpdateMenuBody final {
    RUVIA_OPTIONAL_FIELD(name, ruvia::String);
    RUVIA_OPTIONAL_FIELD(path, ruvia::String);
    RUVIA_OPTIONAL_FIELD(icon, ruvia::String);
    RUVIA_OPTIONAL_FIELD(component, ruvia::String);
    RUVIA_OPTIONAL_FIELD_NAME("parent_id", parentId, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD_NAME("sort_order", sortOrder, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD(type, ruvia::String);
    RUVIA_OPTIONAL_FIELD(status, ruvia::String);
    RUVIA_OPTIONAL_FIELD_NAME("permission_code", permissionCode, ruvia::String);
    RUVIA_OPTIONAL_FIELD_NAME("is_default", isDefault, ruvia::Bool);
    RUVIA_MODEL(UpdateMenuBody, name, path, icon, component, parentId, sortOrder, type, status,
                permissionCode, isDefault);
};

struct ReorderMenuItemBody final {
    RUVIA_OPTIONAL_FIELD(id, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD_NAME("sort_order", sortOrder, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD_NAME("parent_id", parentId, ruvia::Int64);
    RUVIA_MODEL(ReorderMenuItemBody, id, sortOrder, parentId);
};

struct ReorderMenuBody final {
    RUVIA_OPTIONAL_FIELD(items, ruvia::Array<ReorderMenuItemBody>);
    RUVIA_MODEL(ReorderMenuBody, items);
};

struct BatchCreateMenuButtonItemBody final {
    RUVIA_OPTIONAL_FIELD(name, ruvia::String);
    RUVIA_OPTIONAL_FIELD_NAME("permission_code", permissionCode, ruvia::String);
    RUVIA_MODEL(BatchCreateMenuButtonItemBody, name, permissionCode);
};

struct BatchCreateMenuButtonsBody final {
    RUVIA_OPTIONAL_FIELD_NAME("parent_id", parentId, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD(items, ruvia::Array<BatchCreateMenuButtonItemBody>);
    RUVIA_MODEL(BatchCreateMenuButtonsBody, parentId, items);
};

struct MenuDto final {
    RUVIA_OPTIONAL_FIELD(id, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD(name, ruvia::String);
    RUVIA_OPTIONAL_FIELD(path, ruvia::String, RUVIA_OMIT_EMPTY);
    RUVIA_OPTIONAL_FIELD(icon, ruvia::String, RUVIA_OMIT_EMPTY);
    RUVIA_OPTIONAL_FIELD_NAME("parent_id", parentId, ruvia::Int64, RUVIA_EMIT_NULL);
    RUVIA_OPTIONAL_FIELD_NAME("sort_order", sortOrder, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD(type, ruvia::String);
    RUVIA_OPTIONAL_FIELD(component, ruvia::String, RUVIA_OMIT_EMPTY);
    RUVIA_OPTIONAL_FIELD(status, ruvia::String);
    RUVIA_OPTIONAL_FIELD_NAME("permission_code", permissionCode, ruvia::String, RUVIA_OMIT_EMPTY);
    RUVIA_OPTIONAL_FIELD_NAME("is_default", isDefault, ruvia::Bool);
    RUVIA_OPTIONAL_FIELD_NAME("full_path", fullPath, ruvia::String, RUVIA_OMIT_EMPTY);
    RUVIA_OPTIONAL_FIELD(children, ruvia::BoxedArray<MenuDto>, RUVIA_OMIT_EMPTY);
    RUVIA_MODEL(MenuDto, id, name, path, icon, parentId, sortOrder, type, component, status,
                permissionCode, isDefault, fullPath, children);
};

struct MenuListResponse final {
    RUVIA_OPTIONAL_FIELD(code, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD(message, ruvia::String);
    RUVIA_OPTIONAL_FIELD(data, ruvia::BoxedArray<MenuDto>);
    RUVIA_MODEL(MenuListResponse, code, message, data);
};

struct MenuPageDataDto final {
    RUVIA_OPTIONAL_FIELD(list, ruvia::BoxedArray<MenuDto>);
    RUVIA_OPTIONAL_FIELD(total, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD(page, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD_NAME("pageSize", pageSize, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD_NAME("totalPages", totalPages, ruvia::Int64);
    RUVIA_MODEL(MenuPageDataDto, list, total, page, pageSize, totalPages);
};

struct MenuPageResponse final {
    RUVIA_OPTIONAL_FIELD(code, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD(message, ruvia::String);
    RUVIA_OPTIONAL_FIELD(data, MenuPageDataDto);
    RUVIA_MODEL(MenuPageResponse, code, message, data);
};

struct MenuDetailResponse final {
    RUVIA_OPTIONAL_FIELD(code, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD(message, ruvia::String);
    RUVIA_OPTIONAL_FIELD(data, MenuDto);
    RUVIA_MODEL(MenuDetailResponse, code, message, data);
};

} // namespace service::menu
