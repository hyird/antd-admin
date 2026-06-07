#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include <cyra/http/Model.h>

#include "service/common/types.h"

namespace service::modules::system::dept {

struct DeptQuery : service::common::PageParams {
    std::optional<std::string> status;
    std::optional<std::int64_t> parent_id;
};

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

CYRA_MODEL(DeptPageDataDto,
    CYRA_FIELD(list, cyra::List<DeptDto>),
    CYRA_FIELD(total, cyra::Int64),
    CYRA_FIELD(page, cyra::Int64),
    CYRA_FIELD_NAME("pageSize", pageSize, cyra::Int64),
    CYRA_FIELD_NAME("totalPages", totalPages, cyra::Int64)
);

CYRA_MODEL(DeptPageResponse,
    CYRA_FIELD(code, cyra::Int64),
    CYRA_FIELD(message, cyra::String),
    CYRA_FIELD(data, DeptPageDataDto)
);

CYRA_MODEL(DeptDetailResponse,
    CYRA_FIELD(code, cyra::Int64),
    CYRA_FIELD(message, cyra::String),
    CYRA_FIELD(data, DeptDto)
);

}  // namespace service::modules::system::dept
