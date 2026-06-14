#pragma once

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>

#include <ruvia/http/Model.h>

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

RUVIA_MODEL(OperationResponse, RUVIA_FIELD(code, ruvia::Int64), RUVIA_FIELD(message, ruvia::String));

RUVIA_MODEL(HealthData, RUVIA_FIELD(status, ruvia::String));

RUVIA_MODEL(HealthResponse, RUVIA_FIELD(code, ruvia::Int64), RUVIA_FIELD(message, ruvia::String),
           RUVIA_FIELD(data, HealthData));

RUVIA_MODEL(CountData, RUVIA_FIELD_NAME("created_count", createdCount, ruvia::Int64));

RUVIA_MODEL(CountResponse, RUVIA_FIELD(code, ruvia::Int64), RUVIA_FIELD(message, ruvia::String),
           RUVIA_FIELD(data, CountData));

} // namespace service::common
