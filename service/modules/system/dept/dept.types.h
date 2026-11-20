#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include <model.h>

namespace service::dept {

RUVIA_MODEL(
    DeptDto,
    RUVIA_OPTIONAL_FIELD(id, ruvia::int64),
    RUVIA_OPTIONAL_FIELD(name, ruvia::string),
    RUVIA_OPTIONAL_FIELD(code, ruvia::string, RUVIA_OMIT_EMPTY),
    RUVIA_OPTIONAL_FIELD_NAME("parent_id", parentId, ruvia::int64, RUVIA_EMIT_NULL),
    RUVIA_OPTIONAL_FIELD_NAME("sort_order", sortOrder, ruvia::int64),
    RUVIA_OPTIONAL_FIELD_NAME("leader_id", leaderId, ruvia::int64, RUVIA_EMIT_NULL),
    RUVIA_OPTIONAL_FIELD(status, ruvia::string),
    RUVIA_OPTIONAL_FIELD(children, ruvia::boxed_array<DeptDto>, RUVIA_OMIT_EMPTY)
);

RUVIA_MODEL(
    DeptListResponse,
    RUVIA_OPTIONAL_FIELD(code, ruvia::int64),
    RUVIA_OPTIONAL_FIELD(message, ruvia::string),
    RUVIA_OPTIONAL_FIELD(data, ruvia::boxed_array<DeptDto>)
);

RUVIA_MODEL(
    DeptPageDataDto,
    RUVIA_OPTIONAL_FIELD(list, ruvia::boxed_array<DeptDto>),
    RUVIA_OPTIONAL_FIELD(total, ruvia::int64),
    RUVIA_OPTIONAL_FIELD(page, ruvia::int64),
    RUVIA_OPTIONAL_FIELD_NAME("page_size", pageSize, ruvia::int64),
    RUVIA_OPTIONAL_FIELD_NAME("total_pages", totalPages, ruvia::int64)
);

RUVIA_MODEL(
    DeptPageResponse,
    RUVIA_OPTIONAL_FIELD(code, ruvia::int64),
    RUVIA_OPTIONAL_FIELD(message, ruvia::string),
    RUVIA_OPTIONAL_FIELD(data, DeptPageDataDto)
);

RUVIA_MODEL(
    DeptDetailResponse,
    RUVIA_OPTIONAL_FIELD(code, ruvia::int64),
    RUVIA_OPTIONAL_FIELD(message, ruvia::string),
    RUVIA_OPTIONAL_FIELD(data, DeptDto)
);

} // namespace service::dept
