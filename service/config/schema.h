#pragma once

#include <array>

#include <ruvia/db/Db.h>

namespace service::config {

inline constexpr std::array<ruvia::DbMigration, 17> kSchemaMigrations{{
    {
        "0001",
        R"sql(SET NAMES utf8mb4)sql",
    },
    {
        "0002",
        R"sql(CREATE TABLE IF NOT EXISTS sys_menu (
    id              INT NOT NULL AUTO_INCREMENT,
    name            VARCHAR(255) NOT NULL,
    path            VARCHAR(255) NULL,
    icon            VARCHAR(255) NULL,
    parent_id       INT NULL,
    `order`         INT NOT NULL DEFAULT 0,
    type            VARCHAR(20) NOT NULL DEFAULT 'menu',
    component       VARCHAR(255) NULL,
    status          VARCHAR(20) NOT NULL DEFAULT 'enabled',
    permission_code VARCHAR(255) NULL,
    is_default      TINYINT(1) NOT NULL DEFAULT 0,
    created_at      DATETIME(6) NOT NULL DEFAULT CURRENT_TIMESTAMP(6),
    updated_at      DATETIME(6) NOT NULL DEFAULT CURRENT_TIMESTAMP(6) ON UPDATE CURRENT_TIMESTAMP(6),
    deleted_at      DATETIME(6) NULL,
    PRIMARY KEY (id),
    INDEX idx_menu_status (status),
    INDEX idx_menu_type (type),
    INDEX idx_menu_parent_type (parent_id, type)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci)sql",
    },
    {
        "0003",
        R"sql(CREATE TABLE IF NOT EXISTS sys_role (
    id          INT NOT NULL AUTO_INCREMENT,
    code        VARCHAR(255) NOT NULL,
    name        VARCHAR(255) NOT NULL,
    description VARCHAR(255) NULL,
    status      VARCHAR(20) NOT NULL DEFAULT 'enabled',
    created_at  DATETIME(6) NOT NULL DEFAULT CURRENT_TIMESTAMP(6),
    updated_at  DATETIME(6) NOT NULL DEFAULT CURRENT_TIMESTAMP(6) ON UPDATE CURRENT_TIMESTAMP(6),
    deleted_at  DATETIME(6) NULL,
    PRIMARY KEY (id),
    UNIQUE KEY uq_role_code (code)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci)sql",
    },
    {
        "0004",
        R"sql(CREATE TABLE IF NOT EXISTS sys_role_menu (
    role_id INT NOT NULL,
    menu_id INT NOT NULL,
    PRIMARY KEY (role_id, menu_id),
    INDEX idx_role_menu_menu (menu_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci)sql",
    },
    {
        "0005",
        R"sql(CREATE TABLE IF NOT EXISTS sys_dept (
    id          INT NOT NULL AUTO_INCREMENT,
    name        VARCHAR(255) NOT NULL,
    code        VARCHAR(255) NULL,
    parent_id   INT NULL,
    `order`     INT NOT NULL DEFAULT 0,
    leader_id   INT NULL,
    status      VARCHAR(20) NOT NULL DEFAULT 'enabled',
    created_at  DATETIME(6) NOT NULL DEFAULT CURRENT_TIMESTAMP(6),
    updated_at  DATETIME(6) NOT NULL DEFAULT CURRENT_TIMESTAMP(6) ON UPDATE CURRENT_TIMESTAMP(6),
    deleted_at  DATETIME(6) NULL,
    PRIMARY KEY (id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci)sql",
    },
    {
        "0006",
        R"sql(CREATE TABLE IF NOT EXISTS sys_user (
    id            INT NOT NULL AUTO_INCREMENT,
    username      VARCHAR(255) NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    nickname      VARCHAR(255) NULL,
    phone         VARCHAR(255) NULL,
    email         VARCHAR(255) NULL,
    dept_id       INT NULL,
    status        VARCHAR(20) NOT NULL DEFAULT 'enabled',
    created_at    DATETIME(6) NOT NULL DEFAULT CURRENT_TIMESTAMP(6),
    updated_at    DATETIME(6) NOT NULL DEFAULT CURRENT_TIMESTAMP(6) ON UPDATE CURRENT_TIMESTAMP(6),
    deleted_at    DATETIME(6) NULL,
    PRIMARY KEY (id),
    UNIQUE KEY uq_user_username (username),
    INDEX idx_user_status (status),
    INDEX idx_user_dept (dept_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci)sql",
    },
    {
        "0007",
        R"sql(CREATE TABLE IF NOT EXISTS sys_user_role (
    user_id INT NOT NULL,
    role_id INT NOT NULL,
    PRIMARY KEY (user_id, role_id),
    INDEX idx_user_role_role (role_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci)sql",
    },
    {
        "0008",
        R"sql(CREATE TABLE IF NOT EXISTS sys_device (
    id              INT NOT NULL AUTO_INCREMENT,
    code            VARCHAR(100) NOT NULL,
    name            VARCHAR(200) NOT NULL,
    data_source     VARCHAR(100) NULL,
    device_type     VARCHAR(100) NULL,
    facility_type   VARCHAR(100) NULL,
    monitoring_type VARCHAR(100) NULL,
    status          VARCHAR(20) NOT NULL DEFAULT 'enabled',
    remark          VARCHAR(500) NULL,
    created_at      DATETIME(6) NOT NULL DEFAULT CURRENT_TIMESTAMP(6),
    updated_at      DATETIME(6) NOT NULL DEFAULT CURRENT_TIMESTAMP(6) ON UPDATE CURRENT_TIMESTAMP(6),
    deleted_at      DATETIME(6) NULL,
    PRIMARY KEY (id),
    UNIQUE KEY uq_device_code (code),
    INDEX idx_device_name (name),
    INDEX idx_device_status (status)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci)sql",
    },
    {
        "0009",
        R"sql(CREATE TABLE IF NOT EXISTS sys_variable (
    id           INT NOT NULL AUTO_INCREMENT,
    device_id    INT NOT NULL,
    name         VARCHAR(200) NOT NULL,
    code         VARCHAR(100) NOT NULL,
    unit         VARCHAR(50) NULL,
    category     VARCHAR(100) NULL,
    description  VARCHAR(500) NULL,
    scale_factor DECIMAL(12, 6) NOT NULL DEFAULT 1,
    enabled      TINYINT NOT NULL DEFAULT 1,
    sort_order   INT NOT NULL DEFAULT 0,
    created_at   DATETIME(6) NOT NULL DEFAULT CURRENT_TIMESTAMP(6),
    updated_at   DATETIME(6) NOT NULL DEFAULT CURRENT_TIMESTAMP(6) ON UPDATE CURRENT_TIMESTAMP(6),
    deleted_at   DATETIME(6) NULL,
    PRIMARY KEY (id),
    UNIQUE KEY uq_variable_code (code),
    INDEX idx_variable_device (device_id),
    INDEX idx_variable_device_sort (device_id, sort_order),
    INDEX idx_variable_enabled (enabled)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci)sql",
    },
    {
        "0010",
        R"sql(CREATE TABLE IF NOT EXISTS sys_variable_history (
    id            INT NOT NULL AUTO_INCREMENT,
    device_id     INT NOT NULL,
    device_code   VARCHAR(100) NOT NULL,
    device_name   VARCHAR(200) NOT NULL,
    variable_name VARCHAR(200) NOT NULL,
    code          VARCHAR(100) NOT NULL,
    value         DECIMAL(12, 4) NULL,
    collect_time  DATETIME NOT NULL,
    created_at    DATETIME(6) NOT NULL DEFAULT CURRENT_TIMESTAMP(6),
    updated_at    DATETIME(6) NOT NULL DEFAULT CURRENT_TIMESTAMP(6) ON UPDATE CURRENT_TIMESTAMP(6),
    deleted_at    DATETIME(6) NULL,
    PRIMARY KEY (id),
    UNIQUE KEY uq_variable_history (code, collect_time),
    INDEX idx_variable_history_device (device_id),
    INDEX idx_variable_history_time (collect_time)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci)sql",
    },
    {
        "0011",
        R"sql(CREATE TABLE IF NOT EXISTS sys_variable_realtime (
    code          VARCHAR(100) NOT NULL,
    device_id     INT NOT NULL,
    device_code   VARCHAR(100) NOT NULL,
    device_name   VARCHAR(200) NOT NULL,
    variable_name VARCHAR(200) NOT NULL,
    value         DECIMAL(12, 4) NULL,
    collect_time  DATETIME NULL,
    updated_at    DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (code),
    INDEX idx_variable_realtime_device (device_id),
    INDEX idx_variable_realtime_time (collect_time)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci)sql",
    },
    {
        "0012",
        R"sql(INSERT INTO sys_role (name, code, status, created_at, updated_at)
SELECT '超级管理员', 'superadmin', 'enabled', NOW(), NOW()
WHERE NOT EXISTS (
    SELECT 1 FROM sys_role WHERE code = 'superadmin' AND deleted_at IS NULL
))sql",
    },
    {
        "0013",
        R"sql(INSERT INTO sys_user (username, password_hash, nickname, status, created_at, updated_at)
SELECT 'admin',
       'pbkdf2_sha256$210000$616e74645f61646d696e5f7365656431$179aef64cb17bf706f7a58feba1ce0db759ee0a63dae18f3789a78a5d7a9614c',
       '超级管理员',
       'enabled',
       NOW(),
       NOW()
WHERE NOT EXISTS (
    SELECT 1 FROM sys_user WHERE username = 'admin' AND deleted_at IS NULL
))sql",
    },
    {
        "0014",
        R"sql(INSERT IGNORE INTO sys_user_role (user_id, role_id)
SELECT u.id, r.id
FROM sys_user u
INNER JOIN sys_role r ON r.code = 'superadmin' AND r.deleted_at IS NULL
WHERE u.username = 'admin' AND u.deleted_at IS NULL)sql",
    },
    {
        "0015",
        R"sql(INSERT INTO sys_menu (name, path, icon, component, parent_id, `order`, type, status, permission_code, is_default, created_at, updated_at)
SELECT '首页', '/home', 'HomeOutlined', 'Home', NULL, 0, 'page', 'enabled', NULL, 1, NOW(), NOW()
WHERE NOT EXISTS (
    SELECT 1 FROM sys_menu WHERE type = 'page' AND component = 'Home' AND deleted_at IS NULL
)
UNION ALL
SELECT '系统管理', NULL, 'SettingOutlined', NULL, NULL, 10, 'menu', 'enabled', NULL, 0, NOW(), NOW()
WHERE NOT EXISTS (
    SELECT 1 FROM sys_menu WHERE type = 'menu' AND name = '系统管理' AND parent_id IS NULL AND deleted_at IS NULL
))sql",
    },
    {
        "0016",
        R"sql(INSERT INTO sys_menu (name, path, icon, component, parent_id, `order`, type, status, permission_code, is_default, created_at, updated_at)
SELECT '用户管理', '/system/user', 'UserOutlined', 'User',
       (SELECT id FROM sys_menu WHERE type = 'menu' AND name = '系统管理' AND deleted_at IS NULL ORDER BY id LIMIT 1),
       10, 'page', 'enabled', NULL, 0, NOW(), NOW()
WHERE NOT EXISTS (
    SELECT 1 FROM sys_menu WHERE type = 'page' AND component = 'User' AND deleted_at IS NULL
)
UNION ALL
SELECT '角色管理', '/system/role', 'SafetyCertificateOutlined', 'Role',
       (SELECT id FROM sys_menu WHERE type = 'menu' AND name = '系统管理' AND deleted_at IS NULL ORDER BY id LIMIT 1),
       20, 'page', 'enabled', NULL, 0, NOW(), NOW()
WHERE NOT EXISTS (
    SELECT 1 FROM sys_menu WHERE type = 'page' AND component = 'Role' AND deleted_at IS NULL
)
UNION ALL
SELECT '部门管理', '/system/dept', 'ApartmentOutlined', 'Dept',
       (SELECT id FROM sys_menu WHERE type = 'menu' AND name = '系统管理' AND deleted_at IS NULL ORDER BY id LIMIT 1),
       30, 'page', 'enabled', NULL, 0, NOW(), NOW()
WHERE NOT EXISTS (
    SELECT 1 FROM sys_menu WHERE type = 'page' AND component = 'Dept' AND deleted_at IS NULL
)
UNION ALL
SELECT '菜单管理', '/system/menu', 'MenuOutlined', 'Menu',
       (SELECT id FROM sys_menu WHERE type = 'menu' AND name = '系统管理' AND deleted_at IS NULL ORDER BY id LIMIT 1),
       40, 'page', 'enabled', NULL, 0, NOW(), NOW()
WHERE NOT EXISTS (
    SELECT 1 FROM sys_menu WHERE type = 'page' AND component = 'Menu' AND deleted_at IS NULL
))sql",
    },
    {
        "0017",
        R"sql(INSERT INTO sys_menu (name, path, icon, component, parent_id, `order`, type, status, permission_code, is_default, created_at, updated_at)
SELECT '查看统计', NULL, NULL, NULL,
       (SELECT id FROM sys_menu WHERE type = 'page' AND component = 'Home' AND deleted_at IS NULL ORDER BY id LIMIT 1),
       1, 'button', 'enabled', 'home:dashboard:query', 0, NOW(), NOW()
WHERE NOT EXISTS (SELECT 1 FROM sys_menu WHERE type = 'button' AND permission_code = 'home:dashboard:query' AND deleted_at IS NULL)
UNION ALL
SELECT '查询用户', NULL, NULL, NULL,
       (SELECT id FROM sys_menu WHERE type = 'page' AND component = 'User' AND deleted_at IS NULL ORDER BY id LIMIT 1),
       1, 'button', 'enabled', 'system:user:query', 0, NOW(), NOW()
WHERE NOT EXISTS (SELECT 1 FROM sys_menu WHERE type = 'button' AND permission_code = 'system:user:query' AND deleted_at IS NULL)
UNION ALL
SELECT '新增用户', NULL, NULL, NULL,
       (SELECT id FROM sys_menu WHERE type = 'page' AND component = 'User' AND deleted_at IS NULL ORDER BY id LIMIT 1),
       2, 'button', 'enabled', 'system:user:add', 0, NOW(), NOW()
WHERE NOT EXISTS (SELECT 1 FROM sys_menu WHERE type = 'button' AND permission_code = 'system:user:add' AND deleted_at IS NULL)
UNION ALL
SELECT '编辑用户', NULL, NULL, NULL,
       (SELECT id FROM sys_menu WHERE type = 'page' AND component = 'User' AND deleted_at IS NULL ORDER BY id LIMIT 1),
       3, 'button', 'enabled', 'system:user:edit', 0, NOW(), NOW()
WHERE NOT EXISTS (SELECT 1 FROM sys_menu WHERE type = 'button' AND permission_code = 'system:user:edit' AND deleted_at IS NULL)
UNION ALL
SELECT '删除用户', NULL, NULL, NULL,
       (SELECT id FROM sys_menu WHERE type = 'page' AND component = 'User' AND deleted_at IS NULL ORDER BY id LIMIT 1),
       4, 'button', 'enabled', 'system:user:delete', 0, NOW(), NOW()
WHERE NOT EXISTS (SELECT 1 FROM sys_menu WHERE type = 'button' AND permission_code = 'system:user:delete' AND deleted_at IS NULL)
UNION ALL
SELECT '查询角色', NULL, NULL, NULL,
       (SELECT id FROM sys_menu WHERE type = 'page' AND component = 'Role' AND deleted_at IS NULL ORDER BY id LIMIT 1),
       1, 'button', 'enabled', 'system:role:query', 0, NOW(), NOW()
WHERE NOT EXISTS (SELECT 1 FROM sys_menu WHERE type = 'button' AND permission_code = 'system:role:query' AND deleted_at IS NULL)
UNION ALL
SELECT '新增角色', NULL, NULL, NULL,
       (SELECT id FROM sys_menu WHERE type = 'page' AND component = 'Role' AND deleted_at IS NULL ORDER BY id LIMIT 1),
       2, 'button', 'enabled', 'system:role:add', 0, NOW(), NOW()
WHERE NOT EXISTS (SELECT 1 FROM sys_menu WHERE type = 'button' AND permission_code = 'system:role:add' AND deleted_at IS NULL)
UNION ALL
SELECT '编辑角色', NULL, NULL, NULL,
       (SELECT id FROM sys_menu WHERE type = 'page' AND component = 'Role' AND deleted_at IS NULL ORDER BY id LIMIT 1),
       3, 'button', 'enabled', 'system:role:edit', 0, NOW(), NOW()
WHERE NOT EXISTS (SELECT 1 FROM sys_menu WHERE type = 'button' AND permission_code = 'system:role:edit' AND deleted_at IS NULL)
UNION ALL
SELECT '删除角色', NULL, NULL, NULL,
       (SELECT id FROM sys_menu WHERE type = 'page' AND component = 'Role' AND deleted_at IS NULL ORDER BY id LIMIT 1),
       4, 'button', 'enabled', 'system:role:delete', 0, NOW(), NOW()
WHERE NOT EXISTS (SELECT 1 FROM sys_menu WHERE type = 'button' AND permission_code = 'system:role:delete' AND deleted_at IS NULL)
UNION ALL
SELECT '分配权限', NULL, NULL, NULL,
       (SELECT id FROM sys_menu WHERE type = 'page' AND component = 'Role' AND deleted_at IS NULL ORDER BY id LIMIT 1),
       5, 'button', 'enabled', 'system:role:perm', 0, NOW(), NOW()
WHERE NOT EXISTS (SELECT 1 FROM sys_menu WHERE type = 'button' AND permission_code = 'system:role:perm' AND deleted_at IS NULL)
UNION ALL
SELECT '查询部门', NULL, NULL, NULL,
       (SELECT id FROM sys_menu WHERE type = 'page' AND component = 'Dept' AND deleted_at IS NULL ORDER BY id LIMIT 1),
       1, 'button', 'enabled', 'system:dept:query', 0, NOW(), NOW()
WHERE NOT EXISTS (SELECT 1 FROM sys_menu WHERE type = 'button' AND permission_code = 'system:dept:query' AND deleted_at IS NULL)
UNION ALL
SELECT '新增部门', NULL, NULL, NULL,
       (SELECT id FROM sys_menu WHERE type = 'page' AND component = 'Dept' AND deleted_at IS NULL ORDER BY id LIMIT 1),
       2, 'button', 'enabled', 'system:dept:add', 0, NOW(), NOW()
WHERE NOT EXISTS (SELECT 1 FROM sys_menu WHERE type = 'button' AND permission_code = 'system:dept:add' AND deleted_at IS NULL)
UNION ALL
SELECT '编辑部门', NULL, NULL, NULL,
       (SELECT id FROM sys_menu WHERE type = 'page' AND component = 'Dept' AND deleted_at IS NULL ORDER BY id LIMIT 1),
       3, 'button', 'enabled', 'system:dept:edit', 0, NOW(), NOW()
WHERE NOT EXISTS (SELECT 1 FROM sys_menu WHERE type = 'button' AND permission_code = 'system:dept:edit' AND deleted_at IS NULL)
UNION ALL
SELECT '删除部门', NULL, NULL, NULL,
       (SELECT id FROM sys_menu WHERE type = 'page' AND component = 'Dept' AND deleted_at IS NULL ORDER BY id LIMIT 1),
       4, 'button', 'enabled', 'system:dept:delete', 0, NOW(), NOW()
WHERE NOT EXISTS (SELECT 1 FROM sys_menu WHERE type = 'button' AND permission_code = 'system:dept:delete' AND deleted_at IS NULL)
UNION ALL
SELECT '查询菜单', NULL, NULL, NULL,
       (SELECT id FROM sys_menu WHERE type = 'page' AND component = 'Menu' AND deleted_at IS NULL ORDER BY id LIMIT 1),
       1, 'button', 'enabled', 'system:menu:query', 0, NOW(), NOW()
WHERE NOT EXISTS (SELECT 1 FROM sys_menu WHERE type = 'button' AND permission_code = 'system:menu:query' AND deleted_at IS NULL)
UNION ALL
SELECT '新增菜单', NULL, NULL, NULL,
       (SELECT id FROM sys_menu WHERE type = 'page' AND component = 'Menu' AND deleted_at IS NULL ORDER BY id LIMIT 1),
       2, 'button', 'enabled', 'system:menu:add', 0, NOW(), NOW()
WHERE NOT EXISTS (SELECT 1 FROM sys_menu WHERE type = 'button' AND permission_code = 'system:menu:add' AND deleted_at IS NULL)
UNION ALL
SELECT '编辑菜单', NULL, NULL, NULL,
       (SELECT id FROM sys_menu WHERE type = 'page' AND component = 'Menu' AND deleted_at IS NULL ORDER BY id LIMIT 1),
       3, 'button', 'enabled', 'system:menu:edit', 0, NOW(), NOW()
WHERE NOT EXISTS (SELECT 1 FROM sys_menu WHERE type = 'button' AND permission_code = 'system:menu:edit' AND deleted_at IS NULL)
UNION ALL
SELECT '删除菜单', NULL, NULL, NULL,
       (SELECT id FROM sys_menu WHERE type = 'page' AND component = 'Menu' AND deleted_at IS NULL ORDER BY id LIMIT 1),
       4, 'button', 'enabled', 'system:menu:delete', 0, NOW(), NOW()
WHERE NOT EXISTS (SELECT 1 FROM sys_menu WHERE type = 'button' AND permission_code = 'system:menu:delete' AND deleted_at IS NULL))sql",
    },
}};

} // namespace service::config
