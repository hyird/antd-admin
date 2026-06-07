#pragma once

#include <charconv>
#include <cstdint>
#include <initializer_list>
#include <optional>
#include <string>
#include <string_view>

#include <cyra/http/Context.h>

#include "service/common/http.h"
#include "service/common/types.h"

namespace service::common {

inline std::optional<std::string> getQuery(cyra::Context& c,
                                           std::string_view key,
                                           std::initializer_list<std::string_view> aliases = {}) {
    if (auto value = c.query(key); value && !value->empty()) {
        return std::string(*value);
    }
    for (const auto alias : aliases) {
        if (auto value = c.query(alias); value && !value->empty()) {
            return std::string(*value);
        }
    }
    return std::nullopt;
}

inline std::optional<std::int64_t> getQueryInt(
    cyra::Context& c,
    std::string_view key,
    std::initializer_list<std::string_view> aliases = {}) {
    auto text = getQuery(c, key, aliases);
    if (!text) return std::nullopt;
    std::int64_t value = 0;
    const auto* first = text->data();
    const auto* last = first + text->size();
    auto [ptr, ec] = std::from_chars(first, last, value);
    if (ec != std::errc{} || ptr != last) return std::nullopt;
    return value;
}

inline PageParams parsePageParams(cyra::Context& c) {
    PageParams params;
    params.page = getQueryInt(c, "page");
    params.page_size = getQueryInt(c, "pageSize", {"page_size"});
    params.keyword = getQuery(c, "keyword");
    return params;
}

inline std::int64_t parseIdParam(cyra::Context& c) {
    const auto raw = c.param("id");
    if (raw.empty()) {
        throwAppError(kValidationErrorCode, "缺少 id 参数", 400);
    }
    std::int64_t value = 0;
    const auto* first = raw.data();
    const auto* last = first + raw.size();
    auto [ptr, ec] = std::from_chars(first, last, value);
    if (ec != std::errc{} || ptr != last || value <= 0) {
        throwAppError(kValidationErrorCode, "id 必须是正整数", 400);
    }
    return value;
}

}  // namespace service::common
