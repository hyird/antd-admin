#include <cstdint>
#include <chrono>
#include <exception>
#include <filesystem>
#include <iostream>
#include <string>
#include <system_error>
#include <utility>

#include <task.h>
#include <http_request.h>
#include <http_status.h>
#include <app.h>
#include <context.h>
#include <controller.h>
#include <http_error.h>
#include <db_executor.h>
#include <db_migration.h>
#include <redis_executor.h>

#include "service/common/http.h"
#include "service/config/schema.h"

// 业务控制器（RUVIA_CONTROLLER_GROUP 在静态阶段把路由表注册到 ruvia::app()）。
#include "service/modules/auth/login/login.controller.h"
#include "service/modules/system/dept/dept.controller.h"
#include "service/modules/system/menu/menu.controller.h"
#include "service/modules/system/role/role.controller.h"
#include "service/modules/system/user/user.controller.h"
#include "service/utils/jwt.h"

namespace {

struct ServerSettings {
    std::string host;
    std::uint16_t port;
    std::size_t threads;
};

std::filesystem::path executableDir(const char* executablePath) {
    if (executablePath == nullptr || executablePath[0] == '\0') {
        return std::filesystem::current_path();
    }

    std::filesystem::path path(executablePath);
    if (path.is_relative()) {
        path = std::filesystem::current_path() / path;
    }

    std::error_code ec;
    auto canonical = std::filesystem::weakly_canonical(path, ec);
    if (!ec) {
        path = std::move(canonical);
    }

    const auto dir = path.parent_path();
    return dir.empty() ? std::filesystem::current_path() : dir;
}

std::filesystem::path& webRootPath() {
    static std::filesystem::path path;
    return path;
}

void configureDocumentRoot(ruvia::application& app, const std::filesystem::path& runtimeDir) {
    auto webRoot = runtimeDir / "web";
    if (!std::filesystem::is_directory(webRoot))
        return;

    ruvia::document_root_config config;
    config.root_ = std::move(webRoot);
    config.static_options_.cache_control_ = "public, max-age=3600";
    webRootPath() = config.root_;
    app.document_root(std::move(config));
}

ruvia::db_config dbConfigFromEnv(const ruvia::env& env) {
    ruvia::db_config config{.driver_ = ruvia::db_driver::mariadb};
    config.host_.assign(env.get("DB_HOST").value_or("127.0.0.1"));
    config.port_ = ruvia::port_number{env.get<int>("DB_PORT").value_or(3306)};
    config.username_.assign(env.get("DB_USERNAME").value_or("root"));
    config.password_.assign(env.get("DB_PASSWORD").value_or(""));
    config.database_.assign(env.get("DB_DATABASE").value_or("antd_admin"));
    if (!env.get<bool>("DB_TLS").value_or(false))
        config.tls_.mode_ = ruvia::client_tls_mode::disabled;
    return config;
}

ruvia::redis_config redisConfigFromEnv(const ruvia::env& env) {
    ruvia::redis_config config;
    config.host_.assign(env.get("REDIS_HOST").value_or("127.0.0.1"));
    config.port_ = ruvia::port_number{env.get<int>("REDIS_PORT").value_or(6379)};
    config.username_.assign(env.get("REDIS_USERNAME").value_or(""));
    config.password_.assign(env.get("REDIS_PASSWORD").value_or(""));
    config.database_ = env.get<std::uint32_t>("REDIS_DATABASE").value_or(0);
    if (!env.get<bool>("REDIS_TLS").value_or(false))
        config.tls_.mode_ = ruvia::client_tls_mode::disabled;
    return config;
}

void configureDatabase(ruvia::application& app) {
    auto dbConfig = dbConfigFromEnv(app.env());
    ruvia::db_migrator::run(dbConfig, service::config::kSchemaMigrations,
                            {.table_ = "cyra_schema_migrations"});
    auto cachePrefix = "antd-admin:" + dbConfig.host_ + ':' +
                       std::to_string(dbConfig.port_->value()) + ':' + dbConfig.database_;
    app.redis({.config_ = redisConfigFromEnv(app.env())})
        .db({.config_ = std::move(dbConfig),
             .query_cache_ = ruvia::db_query_cache_registration{
                 .policy_ = {.duration_ = std::chrono::seconds(60),
                             .ignore_errors_ = true,
                             .key_prefix_ = std::move(cachePrefix)}}});
}

ServerSettings serverSettingsFromEnv(const ruvia::env& env) {
    return {
        .host = std::string(env.get("HOST").value_or("0.0.0.0")),
        .port = static_cast<std::uint16_t>(env.get<int>("PORT").value_or(1102)),
        .threads = static_cast<std::size_t>(env.get<int>("WORKER_THREADS").value_or(2)),
    };
}

bool isSpaFallbackRequest(ruvia::context& c, ruvia::http_error_info info) {
    if (info.status() != ruvia::http_status::not_found || webRootPath().empty())
        return false;
    if (c.request().known_method() != ruvia::http_known_method::get &&
        c.request().known_method() != ruvia::http_known_method::head)
        return false;

    auto path = c.request().path();
    if (path.starts_with("/api/") || path == "/api")
        return false;
    if (path.starts_with("/assets/"))
        return false;

    const auto lastSlash = path.rfind('/');
    const auto lastSegment =
        lastSlash == std::string_view::npos ? path : path.substr(lastSlash + 1);
    return lastSegment.find('.') == std::string_view::npos;
}

// 与 ruvia::http_error_handler 签名匹配；运行时把业务 HttpError 转成统一响应 DTO。
ruvia::task<ruvia::http_response> handleError(ruvia::context& c, ruvia::http_error_info info) {
    if (isSpaFallbackRequest(c, info)) {
        const auto index = webRootPath() / "index.html";
        co_return co_await c.file({.path_ = index, .content_type_ = "text/html; charset=utf-8"});
    }

    const auto status = info.status();
    const auto issues = info.validation_issues();
    const auto message =
        issues.empty()
            ? (info.message().empty() ? ruvia::http_reason_phrase(status) : info.message())
            : issues.front().message();
    co_return c.json(
        service::common::error(
            service::common::normalizeBusinessErrorCode(info.code(), status.value()), message
        ),
        {.status_ = status}
    );
}

ruvia::task<ruvia::http_response> handleNotFound(ruvia::context& c) {
    return handleError(c, ruvia::http_error_info({.status_ = ruvia::http_status::not_found}));
}

void configureHttpServer(ruvia::application& app) {
    const auto settings = serverSettingsFromEnv(app.env());
    app.listen({.address_ = settings.host, .http_ = ruvia::port_number{settings.port}})
        .workers({.worker_count_ = settings.threads,
                  .process_signal_handlers_ = ruvia::process_signal_handler_policy::install})
        .blocking_pool(ruvia::blocking_pool_options{})
        .on_error(&handleError)
        .on_not_found(&handleNotFound);
}

} // namespace

// 健康检查。
class HealthController final {
  public:
    RUVIA_ROUTES_BEGIN(HealthController)
    RUVIA_GET("/api/health", health);
    RUVIA_ROUTES_END

  private:
    ruvia::task<ruvia::http_response> health(ruvia::context& c) {
        co_return c.json(service::common::health());
    }
};

int main(int argc, char* argv[]) {
    try {
        auto& app = ruvia::app();
        app.load_dotenv();
        service::utils::validateJwtConfiguration();
        configureDocumentRoot(app, executableDir(argc > 0 ? argv[0] : nullptr));
        configureDatabase(app);
        configureHttpServer(app);
        app.run();
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Server failed: " << ex.what() << '\n';
        return 1;
    }
}
