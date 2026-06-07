#pragma once

#include <filesystem>
#include <initializer_list>

namespace service::config {

inline std::filesystem::path sourceRoot() {
#ifdef ANTD_ADMIN_SOURCE_DIR
    return std::filesystem::path(ANTD_ADMIN_SOURCE_DIR);
#else
    return std::filesystem::current_path();
#endif
}

inline std::filesystem::path firstExistingPath(std::initializer_list<std::filesystem::path> candidates) {
    std::filesystem::path fallback;
    for (const auto& candidate : candidates) {
        fallback = candidate;
        if (std::filesystem::exists(candidate)) return candidate;
    }
    return fallback;
}

inline std::filesystem::path envPath() {
    return firstExistingPath({
        std::filesystem::current_path() / ".env",
        sourceRoot() / ".env",
    });
}

inline std::filesystem::path webRootPath() {
    return firstExistingPath({
        std::filesystem::current_path() / "dist" / "web",
        sourceRoot() / "dist" / "web",
    });
}

}  // namespace service::config
