#pragma once

#include <optional>
#include <string>

#include <model.h>

namespace service::role {

RUVIA_MODEL(RoleItemDto, RUVIA_OPTIONAL_FIELD(id, ruvia::int64),
            RUVIA_OPTIONAL_FIELD(name, ruvia::string), RUVIA_OPTIONAL_FIELD(code, ruvia::string),
            RUVIA_OPTIONAL_FIELD(status, ruvia::string),
            RUVIA_OPTIONAL_FIELD_NAME("menu_ids", menuIds, ruvia::array<ruvia::int64>,
                                      RUVIA_OMIT_EMPTY));

RUVIA_MODEL(RoleMenuDto, RUVIA_OPTIONAL_FIELD(id, ruvia::int64),
            RUVIA_OPTIONAL_FIELD(name, ruvia::string), RUVIA_OPTIONAL_FIELD(type, ruvia::string),
            RUVIA_OPTIONAL_FIELD_NAME("parent_id", parentId, ruvia::int64, RUVIA_EMIT_NULL));

RUVIA_MODEL(RoleDetailDto, RUVIA_OPTIONAL_FIELD(id, ruvia::int64),
            RUVIA_OPTIONAL_FIELD(name, ruvia::string), RUVIA_OPTIONAL_FIELD(code, ruvia::string),
            RUVIA_OPTIONAL_FIELD(status, ruvia::string),
            RUVIA_OPTIONAL_FIELD_NAME("menu_ids", menuIds, ruvia::array<ruvia::int64>),
            RUVIA_OPTIONAL_FIELD(menus, ruvia::array<RoleMenuDto>));

RUVIA_MODEL(RoleOptionDto, RUVIA_OPTIONAL_FIELD(id, ruvia::int64),
            RUVIA_OPTIONAL_FIELD(name, ruvia::string), RUVIA_OPTIONAL_FIELD(code, ruvia::string));

RUVIA_MODEL(RolePageDataDto, RUVIA_OPTIONAL_FIELD(list, ruvia::array<RoleItemDto>),
            RUVIA_OPTIONAL_FIELD(total, ruvia::int64), RUVIA_OPTIONAL_FIELD(page, ruvia::int64),
            RUVIA_OPTIONAL_FIELD_NAME("page_size", pageSize, ruvia::int64),
            RUVIA_OPTIONAL_FIELD_NAME("total_pages", totalPages, ruvia::int64));

RUVIA_MODEL(RolePageResponse, RUVIA_OPTIONAL_FIELD(code, ruvia::int64),
            RUVIA_OPTIONAL_FIELD(message, ruvia::string),
            RUVIA_OPTIONAL_FIELD(data, RolePageDataDto));

RUVIA_MODEL(RoleDetailResponse, RUVIA_OPTIONAL_FIELD(code, ruvia::int64),
            RUVIA_OPTIONAL_FIELD(message, ruvia::string),
            RUVIA_OPTIONAL_FIELD(data, RoleDetailDto));

RUVIA_MODEL(RoleOptionsResponse, RUVIA_OPTIONAL_FIELD(code, ruvia::int64),
            RUVIA_OPTIONAL_FIELD(message, ruvia::string),
            RUVIA_OPTIONAL_FIELD(data, ruvia::array<RoleOptionDto>));

} // namespace service::role
