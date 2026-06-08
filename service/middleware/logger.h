#pragma once

#include <chrono>
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <string_view>

#include <cyra/app/Task.h>
#include <cyra/http/Context.h>
#include <cyra/http/Controller.h>
#include <cyra/http/Error.h>
#include <cyra/http/HttpTypes.h>

namespace service::middleware {

inline std::mutex& loggerMutex() {
    static std::mutex mutex;
    return mutex;
}

inline std::string loggerTimestamp() {
    using namespace std::chrono;
    const auto seconds = time_point_cast<std::chrono::seconds>(system_clock::now());
    const auto time = system_clock::to_time_t(seconds);
    std::tm local{};
#if defined(_WIN32)
    localtime_s(&local, &time);
#else
    localtime_r(&time, &local);
#endif

    std::ostringstream out;
    out << std::put_time(&local, "%Y-%m-%d %H:%M:%S");
    return out.str();
}

inline void writeLogLine(std::ostream& out, std::string_view level, std::string_view message) {
    std::lock_guard<std::mutex> lock(loggerMutex());
    out << '[' << loggerTimestamp() << "] " << level << ' ' << message << '\n';
}

inline void logInfo(std::string_view message) { writeLogLine(std::cout, "INFO", message); }

inline void logError(std::string_view message) { writeLogLine(std::cerr, "ERROR", message); }

inline void logRequest(cyra::Context& c, std::uint16_t statusCode,
                       std::chrono::steady_clock::time_point startedAt) noexcept {
    try {
        const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - startedAt);

        std::ostringstream message;
        const auto remote = c.remoteAddress();
        message << (remote.empty() ? "-" : remote) << ' ' << cyra::methodName(c.req().method())
                << ' ' << c.req().target() << ' ' << statusCode << ' ' << elapsed.count() / 1000
                << '.' << std::setfill('0') << std::setw(3) << elapsed.count() % 1000 << "ms";

        if (statusCode >= 500) {
            logError(message.str());
        } else {
            logInfo(message.str());
        }
    } catch (...) {
    }
}

class LoggerMiddleware final : public cyra::Middleware<LoggerMiddleware> {
  public:
    cyra::Task<cyra::HttpResponse> handle(cyra::Context& c, const cyra::Next& next) {
        const auto startedAt = std::chrono::steady_clock::now();
        try {
            auto response = co_await next(c);
            logRequest(c, response.statusCode(), startedAt);
            co_return response;
        } catch (const cyra::HttpError& error) {
            logRequest(c, error.info().statusCode, startedAt);
            throw;
        } catch (...) {
            logRequest(c, 500, startedAt);
            throw;
        }
    }
};

} // namespace service::middleware
