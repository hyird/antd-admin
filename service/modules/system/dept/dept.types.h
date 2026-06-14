#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include <ruvia/http/Model.h>

namespace service::dept {

RUVIA_MODEL(CreateDeptBody, RUVIA_FIELD(name, ruvia::String), RUVIA_FIELD(code, ruvia::String),
           RUVIA_FIELD_NAME("parent_id", parentId, ruvia::Int64),
           RUVIA_FIELD_NAME("sort_order", sortOrder, ruvia::Int64),
           RUVIA_FIELD_NAME("leader_id", leaderId, ruvia::Int64), RUVIA_FIELD(status, ruvia::String));

RUVIA_MODEL(UpdateDeptBody, RUVIA_FIELD(name, ruvia::String), RUVIA_FIELD(code, ruvia::String),
           RUVIA_FIELD_NAME("parent_id", parentId, ruvia::Int64),
           RUVIA_FIELD_NAME("sort_order", sortOrder, ruvia::Int64),
           RUVIA_FIELD_NAME("leader_id", leaderId, ruvia::Int64), RUVIA_FIELD(status, ruvia::String));

RUVIA_MODEL(DeptDto, RUVIA_FIELD(id, ruvia::Int64), RUVIA_FIELD(name, ruvia::String),
           RUVIA_FIELD(code, ruvia::String, RUVIA_OMIT_EMPTY),
           RUVIA_FIELD_NAME("parent_id", parentId, ruvia::Int64, RUVIA_EMIT_NULL),
           RUVIA_FIELD_NAME("sort_order", sortOrder, ruvia::Int64),
           RUVIA_FIELD_NAME("leader_id", leaderId, ruvia::Int64, RUVIA_EMIT_NULL),
           RUVIA_FIELD(status, ruvia::String),
           RUVIA_FIELD(children, ruvia::List<DeptDto>, RUVIA_OMIT_EMPTY));

RUVIA_MODEL(DeptListResponse, RUVIA_FIELD(code, ruvia::Int64), RUVIA_FIELD(message, ruvia::String),
           RUVIA_FIELD(data, ruvia::List<DeptDto>));

RUVIA_MODEL(DeptPageDataDto, RUVIA_FIELD(list, ruvia::List<DeptDto>), RUVIA_FIELD(total, ruvia::Int64),
           RUVIA_FIELD(page, ruvia::Int64), RUVIA_FIELD_NAME("pageSize", pageSize, ruvia::Int64),
           RUVIA_FIELD_NAME("totalPages", totalPages, ruvia::Int64));

RUVIA_MODEL(DeptPageResponse, RUVIA_FIELD(code, ruvia::Int64), RUVIA_FIELD(message, ruvia::String),
           RUVIA_FIELD(data, DeptPageDataDto));

RUVIA_MODEL(DeptDetailResponse, RUVIA_FIELD(code, ruvia::Int64), RUVIA_FIELD(message, ruvia::String),
           RUVIA_FIELD(data, DeptDto));

} // namespace service::dept
