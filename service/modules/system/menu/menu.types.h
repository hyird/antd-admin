#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include <model.h>

namespace service::menu {

RUVIA_MODEL(
    MenuDto,
    RUVIA_OPTIONAL_FIELD(id, ruvia::int64),
    RUVIA_OPTIONAL_FIELD(name, ruvia::string),
    RUVIA_OPTIONAL_FIELD(path, ruvia::string, RUVIA_OMIT_EMPTY),
    RUVIA_OPTIONAL_FIELD(icon, ruvia::string, RUVIA_OMIT_EMPTY),
    RUVIA_OPTIONAL_FIELD_NAME("parent_id", parentId, ruvia::int64, RUVIA_EMIT_NULL),
    RUVIA_OPTIONAL_FIELD_NAME("sort_order", sortOrder, ruvia::int64),
    RUVIA_OPTIONAL_FIELD(type, ruvia::string),
    RUVIA_OPTIONAL_FIELD(component, ruvia::string, RUVIA_OMIT_EMPTY),
    RUVIA_OPTIONAL_FIELD(status, ruvia::string),
    RUVIA_OPTIONAL_FIELD_NAME("permission_code", permissionCode, ruvia::string, RUVIA_OMIT_EMPTY),
    RUVIA_OPTIONAL_FIELD_NAME("is_default", isDefault, ruvia::bool_value),
    RUVIA_OPTIONAL_FIELD_NAME("full_path", fullPath, ruvia::string, RUVIA_OMIT_EMPTY),
    RUVIA_OPTIONAL_FIELD(children, ruvia::boxed_array<MenuDto>, RUVIA_OMIT_EMPTY)
);

RUVIA_MODEL(
    MenuListResponse,
    RUVIA_OPTIONAL_FIELD(code, ruvia::int64),
    RUVIA_OPTIONAL_FIELD(message, ruvia::string),
    RUVIA_OPTIONAL_FIELD(data, ruvia::boxed_array<MenuDto>)
);

RUVIA_MODEL(
    MenuPageDataDto,
    RUVIA_OPTIONAL_FIELD(list, ruvia::boxed_array<MenuDto>),
    RUVIA_OPTIONAL_FIELD(total, ruvia::int64),
    RUVIA_OPTIONAL_FIELD(page, ruvia::int64),
    RUVIA_OPTIONAL_FIELD_NAME("page_size", pageSize, ruvia::int64),
    RUVIA_OPTIONAL_FIELD_NAME("total_pages", totalPages, ruvia::int64)
);

RUVIA_MODEL(
    MenuPageResponse,
    RUVIA_OPTIONAL_FIELD(code, ruvia::int64),
    RUVIA_OPTIONAL_FIELD(message, ruvia::string),
    RUVIA_OPTIONAL_FIELD(data, MenuPageDataDto)
);

RUVIA_MODEL(
    MenuDetailResponse,
    RUVIA_OPTIONAL_FIELD(code, ruvia::int64),
    RUVIA_OPTIONAL_FIELD(message, ruvia::string),
    RUVIA_OPTIONAL_FIELD(data, MenuDto)
);

} // namespace service::menu
