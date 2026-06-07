#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include <cyra/http/Model.h>

#include "service/common/types.h"

namespace service::modules::system::menu {

struct MenuQuery : service::common::PageParams {
    std::optional<std::string> status;
    std::optional<std::int64_t> parent_id;
};

CYRA_MODEL(CreateMenuBody,
    CYRA_FIELD(name, cyra::String,
        CYRA_REQUIRED("菜单名称不能为空"),
        CYRA_MIN(1, "菜单名称不能为空")),
    CYRA_FIELD(path, cyra::String),
    CYRA_FIELD(icon, cyra::String),
    CYRA_FIELD(component, cyra::String),
    CYRA_FIELD_NAME("parent_id", parentId, cyra::Int64),
    CYRA_FIELD_NAME("sort_order", sortOrder, cyra::Int64),
    CYRA_FIELD(type, cyra::String),
    CYRA_FIELD(status, cyra::String),
    CYRA_FIELD_NAME("permission_code", permissionCode, cyra::String),
    CYRA_FIELD_NAME("is_default", isDefault, cyra::Bool)
);

CYRA_MODEL(UpdateMenuBody,
    CYRA_FIELD(name, cyra::String),
    CYRA_FIELD(path, cyra::String),
    CYRA_FIELD(icon, cyra::String),
    CYRA_FIELD(component, cyra::String),
    CYRA_FIELD_NAME("parent_id", parentId, cyra::Int64),
    CYRA_FIELD_NAME("sort_order", sortOrder, cyra::Int64),
    CYRA_FIELD(type, cyra::String),
    CYRA_FIELD(status, cyra::String),
    CYRA_FIELD_NAME("permission_code", permissionCode, cyra::String),
    CYRA_FIELD_NAME("is_default", isDefault, cyra::Bool)
);

CYRA_MODEL(ReorderMenuItemBody,
    CYRA_FIELD(id, cyra::Int64, CYRA_REQUIRED("菜单 id 不能为空")),
    CYRA_FIELD_NAME("sort_order", sortOrder, cyra::Int64, CYRA_REQUIRED("排序不能为空")),
    CYRA_FIELD_NAME("parent_id", parentId, cyra::Int64)
);

CYRA_MODEL(ReorderMenuBody,
    CYRA_FIELD(items, cyra::Array<ReorderMenuItemBody>,
        CYRA_REQUIRED("排序项不能为空"),
        CYRA_MIN(1, "排序项不能为空"))
);

CYRA_MODEL(BatchCreateMenuButtonItemBody,
    CYRA_FIELD(name, cyra::String,
        CYRA_REQUIRED("按钮名称不能为空"),
        CYRA_MIN(1, "按钮名称不能为空")),
    CYRA_FIELD_NAME("permission_code", permissionCode, cyra::String,
        CYRA_REQUIRED("权限编码不能为空"),
        CYRA_MIN(1, "权限编码不能为空"))
);

CYRA_MODEL(BatchCreateMenuButtonsBody,
    CYRA_FIELD_NAME("parent_id", parentId, cyra::Int64, CYRA_REQUIRED("父级菜单不能为空")),
    CYRA_FIELD(items, cyra::Array<BatchCreateMenuButtonItemBody>,
        CYRA_REQUIRED("按钮不能为空"),
        CYRA_MIN(1, "按钮不能为空"))
);

CYRA_MODEL(MenuDto,
    CYRA_FIELD(id, cyra::Int64),
    CYRA_FIELD(name, cyra::String),
    CYRA_FIELD(path, cyra::String, CYRA_OMIT_EMPTY),
    CYRA_FIELD(icon, cyra::String, CYRA_OMIT_EMPTY),
    CYRA_FIELD_NAME("parent_id", parentId, cyra::Int64, CYRA_EMIT_NULL),
    CYRA_FIELD_NAME("sort_order", sortOrder, cyra::Int64),
    CYRA_FIELD(type, cyra::String),
    CYRA_FIELD(component, cyra::String, CYRA_OMIT_EMPTY),
    CYRA_FIELD(status, cyra::String),
    CYRA_FIELD_NAME("permission_code", permissionCode, cyra::String, CYRA_OMIT_EMPTY),
    CYRA_FIELD_NAME("is_default", isDefault, cyra::Bool),
    CYRA_FIELD_NAME("full_path", fullPath, cyra::String, CYRA_OMIT_EMPTY),
    CYRA_FIELD(children, cyra::List<MenuDto>, CYRA_OMIT_EMPTY)
);

CYRA_MODEL(MenuListResponse,
    CYRA_FIELD(code, cyra::Int64),
    CYRA_FIELD(message, cyra::String),
    CYRA_FIELD(data, cyra::List<MenuDto>)
);

CYRA_MODEL(MenuPageDataDto,
    CYRA_FIELD(list, cyra::List<MenuDto>),
    CYRA_FIELD(total, cyra::Int64),
    CYRA_FIELD(page, cyra::Int64),
    CYRA_FIELD_NAME("pageSize", pageSize, cyra::Int64),
    CYRA_FIELD_NAME("totalPages", totalPages, cyra::Int64)
);

CYRA_MODEL(MenuPageResponse,
    CYRA_FIELD(code, cyra::Int64),
    CYRA_FIELD(message, cyra::String),
    CYRA_FIELD(data, MenuPageDataDto)
);

CYRA_MODEL(MenuDetailResponse,
    CYRA_FIELD(code, cyra::Int64),
    CYRA_FIELD(message, cyra::String),
    CYRA_FIELD(data, MenuDto)
);

}  // namespace service::modules::system::menu
