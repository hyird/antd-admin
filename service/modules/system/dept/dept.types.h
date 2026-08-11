#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include <ruvia/web/Model.h>

namespace service::dept {

struct CreateDeptBody final {
    RUVIA_OPTIONAL_FIELD(name, ruvia::String);
    RUVIA_OPTIONAL_FIELD(code, ruvia::String);
    RUVIA_OPTIONAL_FIELD_NAME("parent_id", parentId, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD_NAME("sort_order", sortOrder, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD_NAME("leader_id", leaderId, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD(status, ruvia::String);
    RUVIA_MODEL(CreateDeptBody, name, code, parentId, sortOrder, leaderId, status);
};

struct UpdateDeptBody final {
    RUVIA_OPTIONAL_FIELD(name, ruvia::String);
    RUVIA_OPTIONAL_FIELD(code, ruvia::String);
    RUVIA_OPTIONAL_FIELD_NAME("parent_id", parentId, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD_NAME("sort_order", sortOrder, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD_NAME("leader_id", leaderId, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD(status, ruvia::String);
    RUVIA_MODEL(UpdateDeptBody, name, code, parentId, sortOrder, leaderId, status);
};

struct DeptDto final {
    RUVIA_OPTIONAL_FIELD(id, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD(name, ruvia::String);
    RUVIA_OPTIONAL_FIELD(code, ruvia::String, RUVIA_OMIT_EMPTY);
    RUVIA_OPTIONAL_FIELD_NAME("parent_id", parentId, ruvia::Int64, RUVIA_EMIT_NULL);
    RUVIA_OPTIONAL_FIELD_NAME("sort_order", sortOrder, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD_NAME("leader_id", leaderId, ruvia::Int64, RUVIA_EMIT_NULL);
    RUVIA_OPTIONAL_FIELD(status, ruvia::String);
    RUVIA_OPTIONAL_FIELD(children, ruvia::BoxedArray<DeptDto>, RUVIA_OMIT_EMPTY);
    RUVIA_MODEL(DeptDto, id, name, code, parentId, sortOrder, leaderId, status, children);
};

struct DeptListResponse final {
    RUVIA_OPTIONAL_FIELD(code, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD(message, ruvia::String);
    RUVIA_OPTIONAL_FIELD(data, ruvia::BoxedArray<DeptDto>);
    RUVIA_MODEL(DeptListResponse, code, message, data);
};

struct DeptPageDataDto final {
    RUVIA_OPTIONAL_FIELD(list, ruvia::BoxedArray<DeptDto>);
    RUVIA_OPTIONAL_FIELD(total, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD(page, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD_NAME("pageSize", pageSize, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD_NAME("totalPages", totalPages, ruvia::Int64);
    RUVIA_MODEL(DeptPageDataDto, list, total, page, pageSize, totalPages);
};

struct DeptPageResponse final {
    RUVIA_OPTIONAL_FIELD(code, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD(message, ruvia::String);
    RUVIA_OPTIONAL_FIELD(data, DeptPageDataDto);
    RUVIA_MODEL(DeptPageResponse, code, message, data);
};

struct DeptDetailResponse final {
    RUVIA_OPTIONAL_FIELD(code, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD(message, ruvia::String);
    RUVIA_OPTIONAL_FIELD(data, DeptDto);
    RUVIA_MODEL(DeptDetailResponse, code, message, data);
};

} // namespace service::dept
