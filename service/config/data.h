#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <utility>

#include <cyra/app/App.h>
#include <cyra/db/Db.h>

#include "service/config/schema.h"

namespace service::config {

inline cyra::DbConfig buildDbConfig() {
    const auto& env = cyra::app().env();
    cyra::DbConfig cfg;
    cfg.host.assign(env.get("DB_HOST").value_or("127.0.0.1"));
    cfg.port = static_cast<std::uint16_t>(env.get<int>("DB_PORT").value_or(3306));
    cfg.username.assign(env.get("DB_USERNAME").value_or("root"));
    cfg.password.assign(env.get("DB_PASSWORD").value_or(""));
    cfg.database.assign(env.get("DB_DATABASE").value_or("antd_admin"));
    cfg.poolSize = 4;
    return cfg;
}

inline cyra::DbMigrationReport runMigrations() {
    cyra::DbMigrationOptions options;
    options.table = "sys_schema_migrations";
    return cyra::DbMigrator::migrate(
        buildDbConfig(),
        std::span<const cyra::DbMigration>(kSchemaMigrations.data(), kSchemaMigrations.size()),
        std::move(options));
}

inline void registerDb(cyra::App& app) {
    app.useDb(buildDbConfig());
}

}  // namespace service::config
