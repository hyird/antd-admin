#pragma once

#include "service/common/http.h"

namespace service::modules::system::dept {

namespace DeptError {
inline constexpr service::common::AppErrorDef NOT_FOUND{"DEPARTMENT_NOT_FOUND", "部门不存在", 404};
inline constexpr service::common::AppErrorDef PARENT_SELF{"DEPARTMENT_PARENT_SELF", "父级部门不能是自己", 400};
inline constexpr service::common::AppErrorDef PARENT_IS_CHILD{"DEPARTMENT_PARENT_IS_CHILD", "父级部门不能是自己的子部门", 400};
inline constexpr service::common::AppErrorDef HAS_CHILDREN{"DEPARTMENT_HAS_CHILDREN", "存在子部门，不能删除", 400};
inline constexpr service::common::AppErrorDef HAS_USERS{"DEPARTMENT_HAS_USERS", "部门下存在用户，不能删除", 400};
inline constexpr service::common::AppErrorDef CODE_EXISTS{"DEPARTMENT_CODE_EXISTS", "部门编码已存在", 400};
}  // namespace DeptError

}  // namespace service::modules::system::dept
