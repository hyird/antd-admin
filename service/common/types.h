#pragma once

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>

#include <ruvia/web/Model.h>

namespace service::common {

inline constexpr std::string_view kSuperAdminRoleCode{"superadmin"};

inline auto normalizePagination(std::optional<std::int64_t> pageInput,
                                std::optional<std::int64_t> pageSizeInput,
                                std::optional<std::string_view> keywordInput) {
    const bool paginated = pageInput.has_value() || pageSizeInput.has_value();
    std::int64_t page = pageInput.value_or(1);
    if (page < 1)
        page = 1;
    std::int64_t size = pageSizeInput.value_or(10);
    if (size < 1)
        size = 10;
    if (size > 100)
        size = 100;

    std::optional<std::string> keyword;

    if (keywordInput) {
        std::string k(*keywordInput);
        auto begin =
            std::find_if_not(k.begin(), k.end(), [](unsigned char ch) { return std::isspace(ch); });
        auto end = std::find_if_not(k.rbegin(), k.rend(), [](unsigned char ch) {
                       return std::isspace(ch);
                   }).base();
        if (begin < end) {
            keyword = std::string(begin, end);
        }
    }
    return std::tuple{page, size, (page - 1) * size, std::move(keyword), paginated};
}

// 转义 LIKE 模式中的特殊字符（\ % _），避免用户输入被当作通配符解释；
// 与参数化查询配合使用（MariaDB LIKE 默认以反斜杠为转义符）。
inline std::string escapeLikePattern(std::string_view input) {
    std::string out;
    out.reserve(input.size());
    for (const char ch : input) {
        if (ch == '\\' || ch == '%' || ch == '_')
            out.push_back('\\');
        out.push_back(ch);
    }
    return out;
}

struct OperationResponse final {
    RUVIA_OPTIONAL_FIELD(code, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD(message, ruvia::String);
    RUVIA_MODEL(OperationResponse, code, message);
};

struct HealthData final {
    RUVIA_OPTIONAL_FIELD(status, ruvia::String);
    RUVIA_MODEL(HealthData, status);
};

struct HealthResponse final {
    RUVIA_OPTIONAL_FIELD(code, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD(message, ruvia::String);
    RUVIA_OPTIONAL_FIELD(data, HealthData);
    RUVIA_MODEL(HealthResponse, code, message, data);
};

struct CountData final {
    RUVIA_OPTIONAL_FIELD_NAME("created_count", createdCount, ruvia::Int64);
    RUVIA_MODEL(CountData, createdCount);
};

struct CountResponse final {
    RUVIA_OPTIONAL_FIELD(code, ruvia::Int64);
    RUVIA_OPTIONAL_FIELD(message, ruvia::String);
    RUVIA_OPTIONAL_FIELD(data, CountData);
    RUVIA_MODEL(CountResponse, code, message, data);
};

} // namespace service::common
