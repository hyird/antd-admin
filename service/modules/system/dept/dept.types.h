#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include <ruvia/web/Model.h>

namespace service::dept {

RUVIA_REQUEST_MODEL(CreateDeptBody, RUVIA_OPTIONAL_FIELD(name, ruvia::String),
                    RUVIA_OPTIONAL_FIELD(code, ruvia::String),
                    RUVIA_OPTIONAL_FIELD_NAME("parent_id", parentId, ruvia::Int64),
                    RUVIA_OPTIONAL_FIELD_NAME("sort_order", sortOrder, ruvia::Int64),
                    RUVIA_OPTIONAL_FIELD_NAME("leader_id", leaderId, ruvia::Int64),
                    RUVIA_OPTIONAL_FIELD(status, ruvia::String));

RUVIA_REQUEST_MODEL(UpdateDeptBody, RUVIA_OPTIONAL_FIELD(name, ruvia::String),
                    RUVIA_OPTIONAL_FIELD(code, ruvia::String),
                    RUVIA_OPTIONAL_FIELD_NAME("parent_id", parentId, ruvia::Int64),
                    RUVIA_OPTIONAL_FIELD_NAME("sort_order", sortOrder, ruvia::Int64),
                    RUVIA_OPTIONAL_FIELD_NAME("leader_id", leaderId, ruvia::Int64),
                    RUVIA_OPTIONAL_FIELD(status, ruvia::String));

RUVIA_RESPONSE_MODEL(
    DeptDto, RUVIA_OPTIONAL_FIELD(id, ruvia::Int64), RUVIA_OPTIONAL_FIELD(name, ruvia::String),
    RUVIA_OPTIONAL_FIELD(code, ruvia::String, RUVIA_OMIT_EMPTY),
    RUVIA_OPTIONAL_FIELD_NAME("parent_id", parentId, ruvia::Int64, RUVIA_EMIT_NULL),
    RUVIA_OPTIONAL_FIELD_NAME("sort_order", sortOrder, ruvia::Int64),
    RUVIA_OPTIONAL_FIELD_NAME("leader_id", leaderId, ruvia::Int64, RUVIA_EMIT_NULL),
    RUVIA_OPTIONAL_FIELD(status, ruvia::String),
    RUVIA_OPTIONAL_FIELD(children, ruvia::BoxedArray<DeptDto>, RUVIA_OMIT_EMPTY));

RUVIA_RESPONSE_MODEL(DeptListResponse, RUVIA_OPTIONAL_FIELD(code, ruvia::Int64),
                     RUVIA_OPTIONAL_FIELD(message, ruvia::String),
                     RUVIA_OPTIONAL_FIELD(data, ruvia::BoxedArray<DeptDto>));

RUVIA_RESPONSE_MODEL(DeptPageDataDto, RUVIA_OPTIONAL_FIELD(list, ruvia::BoxedArray<DeptDto>),
                     RUVIA_OPTIONAL_FIELD(total, ruvia::Int64),
                     RUVIA_OPTIONAL_FIELD(page, ruvia::Int64),
                     RUVIA_OPTIONAL_FIELD_NAME("page_size", pageSize, ruvia::Int64),
                     RUVIA_OPTIONAL_FIELD_NAME("total_pages", totalPages, ruvia::Int64));

RUVIA_RESPONSE_MODEL(DeptPageResponse, RUVIA_OPTIONAL_FIELD(code, ruvia::Int64),
                     RUVIA_OPTIONAL_FIELD(message, ruvia::String),
                     RUVIA_OPTIONAL_FIELD(data, DeptPageDataDto));

RUVIA_RESPONSE_MODEL(DeptDetailResponse, RUVIA_OPTIONAL_FIELD(code, ruvia::Int64),
                     RUVIA_OPTIONAL_FIELD(message, ruvia::String),
                     RUVIA_OPTIONAL_FIELD(data, DeptDto));

} // namespace service::dept
