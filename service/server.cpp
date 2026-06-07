#include <atomic>
#include <exception>
#include <filesystem>
#include <iostream>
#include <optional>
#include <utility>

#include <cyra/app/App.h>
#include <cyra/http/Context.h>
#include <cyra/http/Controller.h>
#include <cyra/http/Error.h>
#include <cyra/http/HttpTypes.h>
#include <cyra/http/StaticFiles.h>

#include "service/common/http.h"
#include "service/config/data.h"
#include "service/config/paths.h"
#include "service/utils/logger.h"

// 业务控制器（CYRA_CONTROLLER_GROUP 在静态阶段把路由表注册到 cyra::app()）。
#include "service/modules/system/auth/auth.controller.h"
#include "service/modules/system/dept/dept.controller.h"
#include "service/modules/system/menu/menu.controller.h"
#include "service/modules/system/role/role.controller.h"
#include "service/modules/system/user/user.controller.h"
#include "service/modules/system/user/user.service.h"

namespace {

std::optional<cyra::StaticRoot>& staticAssets() {
    static std::optional<cyra::StaticRoot> assets;
    return assets;
}

void initStaticAssets() {
    const auto webRoot = service::config::webRootPath();
    if (!std::filesystem::is_directory(webRoot)) return;
    cyra::StaticRootOptions options;
    options.indexFile = "index.html";
    options.cacheControl = "public, max-age=3600";
    staticAssets().emplace(webRoot, std::move(options));
}

std::atomic_bool gAdminSeeded{false};

}  // namespace

// 健康检查 + 首次请求时延迟执行 seedAdmin。
class HealthController final : public cyra::Controller<HealthController> {
public:
    CYRA_ROUTES_BEGIN
    CYRA_GET("/api/health", health);
    CYRA_ROUTES_END

private:
    cyra::Task<cyra::HttpResponse> health(cyra::Context& c) {
        bool expected = false;
        if (gAdminSeeded.compare_exchange_strong(expected, true)) {
            try {
                co_await service::modules::system::user::userService().seedAdmin(c);
            } catch (const std::exception& ex) {
                gAdminSeeded.store(false);
                service::utils::logError(std::string("seedAdmin failed: ") + ex.what());
            }
        }
        co_return c.json(service::common::health(c));
    }
};

// 提供 dist/web 静态资源 + SPA 入口回退。
class StaticAssetsController final : public cyra::Controller<StaticAssetsController> {
public:
    CYRA_ROUTES_BEGIN
    CYRA_GET("/", root);
    CYRA_GET("/assets/*", assets);
    CYRA_GET("/*", fallback);
    CYRA_ROUTES_END

private:
    cyra::Task<cyra::HttpResponse> root(cyra::Context& c) {
        if (!staticAssets()) {
            co_return c.text("server running, web bundle not built\n");
        }
        co_return c.staticFile(*staticAssets(), "");
    }

    cyra::Task<cyra::HttpResponse> assets(cyra::Context& c) {
        if (!staticAssets()) {
            co_return c.error(404, "not_found", "static bundle missing");
        }
        co_return c.staticFile(*staticAssets(), c.param("*"));
    }

    cyra::Task<cyra::HttpResponse> fallback(cyra::Context& c) {
        const auto path = c.param("*");
        if (path.starts_with("api/")) {
            co_return c.error(404, "not_found", "no such api route");
        }
        if (!staticAssets()) {
            co_return c.text("server running\n");
        }
        // SPA: 任意非资源路径回退到 index.html。
        co_return c.staticFile(*staticAssets(), "");
    }
};

namespace {

// 与 cyra::HttpErrorHandler 签名匹配；运行时把业务 HttpError 转成统一响应 DTO。
cyra::Task<cyra::HttpResponse> handleError(cyra::Context& c, cyra::HttpErrorInfo info) {
    if (info.statusCode >= 500) {
        service::utils::logError(std::string("Unhandled error: ") + std::string(info.message));
    }
    co_return c.status(info.statusCode).json(
        service::common::error(
            c,
            info.code.empty() ? cyra::defaultErrorCode(info.statusCode) : info.code,
            info.message.empty() ? cyra::defaultStatusText(info.statusCode) : info.message));
}

}  // namespace

int main() {
    try {
        auto& app = cyra::app();
        app.loadDotenv(service::config::envPath());
        initStaticAssets();

        auto migrationReport = service::config::runMigrations();
        service::utils::logInfo(
            "DB migrations applied=" + std::to_string(migrationReport.applied().size()) +
            ", skipped=" + std::to_string(migrationReport.skipped().size()));

        service::config::registerDb(app);

        const auto& env = app.env();
        const auto host = env.get("HOST").value_or("0.0.0.0");
        const auto port = static_cast<std::uint16_t>(env.get<int>("PORT").value_or(1102));
        const auto threads = static_cast<std::size_t>(env.get<int>("WORKER_THREADS").value_or(2));

        app.setListenAddress(host, port)
            .setThreadNum(threads)
            .setErrorHandler(&handleError);

        service::utils::logInfo("Server starting on " + std::string(host) + ":" + std::to_string(port));
        app.run();
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Server failed: " << ex.what() << '\n';
        return 1;
    }
}
