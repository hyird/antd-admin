#pragma once

#include "service/common/http.h"

namespace service::menu {

struct MenuError {
    static inline constexpr service::common::AppErrorDef MENU_NOT_FOUND{14001, "菜单不存在", 404};
    static inline constexpr service::common::AppErrorDef MENU_PARENT_SELF{
        14002, "父级菜单不能是自己", 400};
    static inline constexpr service::common::AppErrorDef MENU_HAS_CHILDREN{
        14003, "存在子菜单，不能删除", 400};
    static inline constexpr service::common::AppErrorDef MENU_PATH_EXISTS{14004, "同级路由已存在",
                                                                          400};
    static inline constexpr service::common::AppErrorDef MENU_PARENT_NOT_FOUND{
        14005, "父级菜单不存在", 400};
    static inline constexpr service::common::AppErrorDef MENU_TYPE_INVALID{
        14006, "菜单节点下只能挂菜单或页面", 400};
    static inline constexpr service::common::AppErrorDef DEFAULT_MUST_BE_PAGE{
        14007, "只有页面类型才能设为默认页面", 400};
    static inline constexpr service::common::AppErrorDef MENU_PARENT_IS_CHILD{
        14008, "父级菜单不能是自己的子菜单", 400};
}; // struct MenuError

} // namespace service::menu
