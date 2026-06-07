#pragma once

#include "service/common/http.h"

namespace service::modules::system::role {

namespace RoleError {
inline constexpr service::common::AppErrorDef CODE_EXISTS{"ROLE_CODE_EXISTS", "角色编码已存在", 400};
inline constexpr service::common::AppErrorDef NOT_FOUND{"ROLE_NOT_FOUND", "角色不存在", 404};
inline constexpr service::common::AppErrorDef SUPERADMIN_CANNOT_DELETE{"ROLE_CANNOT_DELETE", "超级管理员角色不可删除", 400};
inline constexpr service::common::AppErrorDef SUPERADMIN_CANNOT_MODIFY{"ROLE_CANNOT_MODIFY", "超级管理员角色编码不可修改", 400};
inline constexpr service::common::AppErrorDef HAS_USERS{"ROLE_HAS_USERS", "该角色下存在用户，不可删除", 400};
}  // namespace RoleError

}  // namespace service::modules::system::role
