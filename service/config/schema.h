#pragma once

#include <array>

#include <cyra/db/Db.h>

namespace service::config {

inline constexpr std::array<cyra::DbMigration, 11> kSchemaMigrations{{
    {
        "schema_1",
        R"sql(SET NAMES utf8mb4)sql",
    },
    {
        "schema_2",
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
        "schema_3",
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
        "schema_4",
        R"sql(CREATE TABLE IF NOT EXISTS sys_role_menu (
    role_id INT NOT NULL,
    menu_id INT NOT NULL,
    PRIMARY KEY (role_id, menu_id),
    INDEX idx_role_menu_menu (menu_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci)sql",
    },
    {
        "schema_5",
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
        "schema_6",
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
        "schema_7",
        R"sql(CREATE TABLE IF NOT EXISTS sys_user_role (
    user_id INT NOT NULL,
    role_id INT NOT NULL,
    PRIMARY KEY (user_id, role_id),
    INDEX idx_user_role_role (role_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci)sql",
    },
    {
        "schema_8",
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
        "schema_9",
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
        "schema_10",
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
        "schema_11",
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
}};

}  // namespace service::config
