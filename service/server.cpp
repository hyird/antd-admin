#include <cstdint>
#include <exception>
#include <filesystem>
#include <iostream>
#include <string>
#include <system_error>
#include <utility>

#include <cyra/app/App.h>
#include <cyra/db/Db.h>
#include <cyra/http/Context.h>
#include <cyra/http/Controller.h>
#include <cyra/http/Error.h>
#include <cyra/http/HttpTypes.h>

#include "service/common/http.h"
#include "service/config/schema.h"
#include "service/utils/logger.h"

// 业务控制器（CYRA_CONTROLLER_GROUP 在静态阶段把路由表注册到 cyra::app()）。
#include "service/modules/system/auth/auth.controller.h"
#include "service/modules/system/dept/dept.controller.h"
#include "service/modules/system/menu/menu.controller.h"
#include "service/modules/system/role/role.controller.h"
#include "service/modules/system/user/user.controller.h"
#include "service/modules/system/user/user.service.h"

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

void configureDocumentRoot(cyra::App& app, const std::filesystem::path& runtimeDir) {
    auto webRoot = runtimeDir / "web";
    if (!std::filesystem::is_directory(webRoot)) return;

    cyra::DocumentRootConfig config;
    config.root = std::move(webRoot);
    config.staticOptions.indexFile = "index.html";
    config.staticOptions.cacheControl = "public, max-age=3600";
    webRootPath() = config.root;
    app.setDocumentRoot(std::move(config));
}

cyra::DbConfig dbConfigFromEnv(const cyra::Env& env) {
    cyra::DbConfig config;
    config.host.assign(env.get("DB_HOST").value_or("127.0.0.1"));
    config.port = static_cast<std::uint16_t>(env.get<int>("DB_PORT").value_or(3306));
    config.username.assign(env.get("DB_USERNAME").value_or("root"));
    config.password.assign(env.get("DB_PASSWORD").value_or(""));
    config.database.assign(env.get("DB_DATABASE").value_or("antd_admin"));
    config.poolSize = 4;
    return config;
}

void logMigrationReport(const cyra::DbMigrationReport& report) {
    service::utils::logInfo(
        "DB migrations applied=" + std::to_string(report.applied().size()) +
        ", skipped=" + std::to_string(report.skipped().size()));
}

void configureDatabase(cyra::App& app) {
    auto dbConfig = dbConfigFromEnv(app.env());
    logMigrationReport(cyra::DbMigrator::migrate(
        dbConfig,
        service::config::kSchemaMigrations));
    app.useDb(std::move(dbConfig));
}

ServerSettings serverSettingsFromEnv(const cyra::Env& env) {
    return {
        .host = std::string(env.get("HOST").value_or("0.0.0.0")),
        .port = static_cast<std::uint16_t>(env.get<int>("PORT").value_or(1102)),
        .threads = static_cast<std::size_t>(env.get<int>("WORKER_THREADS").value_or(2)),
    };
}

bool isSpaFallbackRequest(cyra::Context& c, cyra::HttpErrorInfo info) {
    if (info.statusCode != 404 || webRootPath().empty()) return false;
    if (c.req().method() != cyra::HttpMethod::kGet && c.req().method() != cyra::HttpMethod::kHead) return false;

    auto path = c.req().path();
    if (path.starts_with("/api/") || path == "/api") return false;
    if (path.starts_with("/assets/")) return false;

    const auto lastSlash = path.rfind('/');
    const auto lastSegment = lastSlash == std::string_view::npos ? path : path.substr(lastSlash + 1);
    return lastSegment.find('.') == std::string_view::npos;
}

// 与 cyra::HttpErrorHandler 签名匹配；运行时把业务 HttpError 转成统一响应 DTO。
cyra::Task<cyra::HttpResponse> handleError(cyra::Context& c, cyra::HttpErrorInfo info) {
    if (isSpaFallbackRequest(c, info)) {
        co_return c.file(webRootPath() / "index.html", "text/html; charset=utf-8");
    }

    if (info.statusCode >= 500) {
        service::utils::logError(std::string("Unhandled error: ") + std::string(info.message));
    }
    co_return c.status(info.statusCode).json(
        service::common::error(
            c,
            service::common::normalizeBusinessErrorCode(info.code, info.statusCode),
            info.message.empty() ? cyra::defaultStatusText(info.statusCode) : info.message));
}

void configureHttpServer(cyra::App& app) {
    const auto settings = serverSettingsFromEnv(app.env());
    app.setListenAddress(settings.host, settings.port)
        .setThreadNum(settings.threads)
        .setErrorHandler(&handleError);
    service::utils::logInfo("Server starting on " + settings.host + ":" + std::to_string(settings.port));
}

}  // namespace

// 健康检查。
class HealthController final : public cyra::Controller<HealthController> {
public:
    CYRA_ROUTES_BEGIN
    CYRA_GET("/api/health", health);
    CYRA_ROUTES_END

private:
    cyra::Task<cyra::HttpResponse> health(cyra::Context& c) {
        co_return c.json(service::common::health(c));
    }
};

int main(int argc, char* argv[]) {
    try {
        auto& app = cyra::app();
        app.loadDotenv();
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
