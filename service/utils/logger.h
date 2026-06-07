#pragma once

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <string_view>

namespace service::utils {

namespace detail {

inline std::mutex& logMutex() {
    static std::mutex mu;
    return mu;
}

inline std::string timestamp() {
    using namespace std::chrono;
    const auto seconds = time_point_cast<std::chrono::seconds>(system_clock::now());
    const auto t = system_clock::to_time_t(seconds);
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

inline void writeLine(std::ostream& out,
                      const std::string& name,
                      std::string_view level,
                      std::string_view msg) {
    std::lock_guard<std::mutex> lock(logMutex());
    out << '[' << timestamp() << "] " << level << ' ';
    if (!name.empty()) out << '[' << name << "] ";
    out << msg << '\n';
}

}  // namespace detail

class Logger {
public:
    explicit Logger(std::string name) : name_(std::move(name)) {}

    void info(std::string_view msg) const { detail::writeLine(std::cout, name_, "INFO", msg); }
    void warn(std::string_view msg) const { detail::writeLine(std::cout, name_, "WARN", msg); }
    void error(std::string_view msg) const { detail::writeLine(std::cerr, name_, "ERROR", msg); }
    void debug(std::string_view msg) const {
#ifndef NDEBUG
        detail::writeLine(std::cout, name_, "DEBUG", msg);
#else
        (void)msg;
#endif
    }

private:
    std::string name_;
};

inline Logger createLogger(std::string name) { return Logger(std::move(name)); }

inline void logInfo(std::string_view msg) { detail::writeLine(std::cout, {}, "INFO", msg); }
inline void logError(std::string_view msg) { detail::writeLine(std::cerr, {}, "ERROR", msg); }

}  // namespace service::utils
