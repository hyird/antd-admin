#pragma once

#include "service/common/http.h"

namespace service::modules::system::role {

namespace RoleError {
inline constexpr service::common::AppErrorDef CODE_EXISTS{13001, "角色编码已存在", 400};
inline constexpr service::common::AppErrorDef NOT_FOUND{13002, "角色不存在", 404};
inline constexpr service::common::AppErrorDef SUPERADMIN_CANNOT_DELETE{
    13003, "超级管理员角色不可删除", 400};
inline constexpr service::common::AppErrorDef SUPERADMIN_CANNOT_MODIFY{
    13004, "超级管理员角色编码不可修改", 400};
inline constexpr service::common::AppErrorDef HAS_USERS{13005, "该角色下存在用户，不可删除", 400};
}  // namespace RoleError

}  // namespace service::modules::system::role
