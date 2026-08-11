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

#include <ruvia/web/ServerConfig.h>

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

// Request access logging runs through App::onAccess. The callback fires once per
// terminal response with the committed status, elapsed time, and connection metadata
// (all borrowed for the call).
inline void logAccess(const ruvia::AccessLogRecord& record) noexcept {
    try {
        const auto micros = record.durationMicros();
        const auto status = record.status();
        std::ostringstream message;
        const auto remote = record.remoteAddress();
        message << (remote.empty() ? "-" : remote) << ' ' << record.method() << ' ' << record.path()
                << ' ' << status.value() << ' ' << micros / 1000 << '.' << std::setfill('0')
                << std::setw(3) << micros % 1000 << "ms";

        if (status.isServerError()) {
            logError(message.str());
        } else {
            logInfo(message.str());
        }
    } catch (...) {
    }
}

// Stored by AccessLogCallback; must be nothrow-invocable.
struct AccessLogger {
    void operator()(const ruvia::AccessLogRecord& record) const noexcept { logAccess(record); }
};

} // namespace service::middleware
