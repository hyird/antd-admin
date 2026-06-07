#pragma once

#include <cyra/http/Model.h>

namespace service::modules::system::dept {

CYRA_MODEL(CreateDeptBody,
    CYRA_FIELD(name, cyra::String,
        CYRA_REQUIRED("部门名称不能为空"),
        CYRA_MIN(1, "部门名称不能为空")),
    CYRA_FIELD(code, cyra::String),
    CYRA_FIELD_NAME("parent_id", parentId, cyra::Int64),
    CYRA_FIELD_NAME("sort_order", sortOrder, cyra::Int64),
    CYRA_FIELD_NAME("leader_id", leaderId, cyra::Int64),
    CYRA_FIELD(status, cyra::String)
);

CYRA_MODEL(UpdateDeptBody,
    CYRA_FIELD(name, cyra::String),
    CYRA_FIELD(code, cyra::String),
    CYRA_FIELD_NAME("parent_id", parentId, cyra::Int64),
    CYRA_FIELD_NAME("sort_order", sortOrder, cyra::Int64),
    CYRA_FIELD_NAME("leader_id", leaderId, cyra::Int64),
    CYRA_FIELD(status, cyra::String)
);

CYRA_MODEL(DeptDto,
    CYRA_FIELD(id, cyra::Int64),
    CYRA_FIELD(name, cyra::String),
    CYRA_FIELD(code, cyra::String, CYRA_OMIT_EMPTY),
    CYRA_FIELD_NAME("parent_id", parentId, cyra::Int64, CYRA_EMIT_NULL),
    CYRA_FIELD_NAME("sort_order", sortOrder, cyra::Int64),
    CYRA_FIELD_NAME("leader_id", leaderId, cyra::Int64, CYRA_EMIT_NULL),
    CYRA_FIELD(status, cyra::String),
    CYRA_FIELD(children, cyra::List<DeptDto>, CYRA_OMIT_EMPTY)
);

CYRA_MODEL(DeptListResponse,
    CYRA_FIELD(code, cyra::Int64),
    CYRA_FIELD(message, cyra::String),
    CYRA_FIELD(data, cyra::List<DeptDto>)
);

CYRA_MODEL(DeptDetailResponse,
    CYRA_FIELD(code, cyra::Int64),
    CYRA_FIELD(message, cyra::String),
    CYRA_FIELD(data, DeptDto)
);

}  // namespace service::modules::system::dept
