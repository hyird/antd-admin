#pragma once

#include "service/common/http.h"

namespace service::modules::system::dept {

namespace DeptError {
inline constexpr service::common::AppErrorDef NOT_FOUND{15001, "部门不存在", 404};
inline constexpr service::common::AppErrorDef PARENT_SELF{15002, "父级部门不能是自己", 400};
inline constexpr service::common::AppErrorDef PARENT_IS_CHILD{15003, "父级部门不能是自己的子部门", 400};
inline constexpr service::common::AppErrorDef HAS_CHILDREN{15004, "存在子部门，不能删除", 400};
inline constexpr service::common::AppErrorDef HAS_USERS{15005, "部门下存在用户，不能删除", 400};
inline constexpr service::common::AppErrorDef CODE_EXISTS{15006, "部门编码已存在", 400};
}  // namespace DeptError

}  // namespace service::modules::system::dept
