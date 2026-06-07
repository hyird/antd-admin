#pragma once

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

#include <cyra/http/Model.h>

namespace service::common {

inline constexpr std::string_view kSuperAdminRoleCode{"1"};

struct PageParams {
    std::optional<std::int64_t> page;
    std::optional<std::int64_t> page_size;
    std::optional<std::string> keyword;
};

struct NormalizedPagination {
    std::int64_t page{1};
    std::int64_t page_size{10};
    std::int64_t skip{0};
    std::optional<std::string> keyword;
    bool paginated{false};
};

inline NormalizedPagination normalizePagination(const PageParams& params) {
    NormalizedPagination out;
    out.paginated = params.page.has_value() || params.page_size.has_value();

    std::int64_t page = params.page.value_or(1);
    if (page < 1) page = 1;
    std::int64_t size = params.page_size.value_or(10);
    if (size < 1) size = 10;
    if (size > 100) size = 100;

    out.page = page;
    out.page_size = size;
    out.skip = (page - 1) * size;

    if (params.keyword) {
        std::string k = *params.keyword;
        auto begin = std::find_if_not(k.begin(), k.end(),
                                      [](unsigned char ch) { return std::isspace(ch); });
        auto end = std::find_if_not(k.rbegin(), k.rend(),
                                    [](unsigned char ch) { return std::isspace(ch); })
                       .base();
        if (begin < end) {
            out.keyword = std::string(begin, end);
        }
    }
    return out;
}

CYRA_MODEL(OperationResponse,
    CYRA_FIELD(code, cyra::Int64),
    CYRA_FIELD(message, cyra::String)
);

CYRA_MODEL(HealthData,
    CYRA_FIELD(status, cyra::String)
);

CYRA_MODEL(HealthResponse,
    CYRA_FIELD(code, cyra::Int64),
    CYRA_FIELD(message, cyra::String),
    CYRA_FIELD(data, HealthData)
);

CYRA_MODEL(CountData,
    CYRA_FIELD_NAME("created_count", createdCount, cyra::Int64)
);

CYRA_MODEL(CountResponse,
    CYRA_FIELD(code, cyra::Int64),
    CYRA_FIELD(message, cyra::String),
    CYRA_FIELD(data, CountData)
);

}  // namespace service::common
