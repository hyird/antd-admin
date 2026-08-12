#pragma once

#include <charconv>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <ruvia/web/Context.h>
#include <ruvia/web/Error.h>
#include <ruvia/web/db/DbTypes.h>

#include "service/common/types.h"

namespace service::common {

// Request query/param/header accessors return std::optional<std::string_view>;
// parse integer inputs strictly at the application boundary.
inline std::optional<std::int64_t> parseInt64(std::optional<std::string_view> input) {
    if (!input || input->empty())
        return std::nullopt;
    std::int64_t value = 0;
    const auto* first = input->data();
    const auto* last = first + input->size();
    const auto [ptr, ec] = std::from_chars(first, last, value);
    if (ec == std::errc{} && ptr == last)
        return value;
    return std::nullopt;
}

// DbHandle::query/execute accept std::span<const ruvia::DbValue>. Build an owning
// vector (which converts to a const span) for inline parameter lists.
// The returned vector and any borrowed argument views live to the end of the
// enclosing co_await full-expression, i.e. across the query's suspension.
template <typename... Ts> inline std::vector<ruvia::DbValue> dbParams(Ts&&... values) {
    std::vector<ruvia::DbValue> params;
    params.reserve(sizeof...(Ts));
    (params.emplace_back(std::forward<Ts>(values)), ...);
    return params;
}

inline constexpr std::int64_t kUnknownErrorCode{10000};
inline constexpr std::int64_t kValidationErrorCode{10001};
inline constexpr std::int64_t kBadRequestErrorCode{10002};
inline constexpr std::int64_t kNotFoundErrorCode{10003};
inline constexpr std::int64_t kServerErrorCode{10004};
inline constexpr std::int64_t kAuthUnauthorizedErrorCode{11004};
inline constexpr std::int64_t kAuthTokenExpiredErrorCode{11005};
inline constexpr std::int64_t kAuthTokenInvalidErrorCode{11006};
inline constexpr std::int64_t kAuthPermissionDeniedErrorCode{11007};

struct AppErrorDef {
    std::int64_t code;
    std::string_view message;
    std::uint16_t status{400};
};

RUVIA_RESPONSE_MODEL(ErrorResponse, RUVIA_OPTIONAL_FIELD(code, ruvia::Int64),
                     RUVIA_OPTIONAL_FIELD(message, ruvia::String));

inline std::int64_t defaultBusinessErrorCode(std::uint16_t status) {
    switch (status) {
    case 400:
    case 422:
        return kBadRequestErrorCode;
    case 401:
        return kAuthUnauthorizedErrorCode;
    case 403:
        return kAuthPermissionDeniedErrorCode;
    case 404:
        return kNotFoundErrorCode;
    case 500:
    case 502:
    case 503:
    case 504:
        return kServerErrorCode;
    default:
        return status >= 500 ? kServerErrorCode : kUnknownErrorCode;
    }
}

inline std::int64_t normalizeBusinessErrorCode(std::string_view code, std::uint16_t status) {
    if (!code.empty()) {
        std::int64_t value = 0;
        const auto* first = code.data();
        const auto* last = first + code.size();
        const auto [ptr, ec] = std::from_chars(first, last, value);
        if (ec == std::errc{} && ptr == last)
            return value;
        if (code == "validation_failed")
            return kValidationErrorCode;
    }
    return defaultBusinessErrorCode(status);
}

[[noreturn]] inline void throwAppError(const AppErrorDef& def) {
    throw ruvia::HttpError(ruvia::HttpStatusCode::fromValue(def.status), std::to_string(def.code),
                           def.message);
}

[[noreturn]] inline void throwAppError(std::int64_t code, std::string message,
                                       std::uint16_t status = 400) {
    throw ruvia::HttpError(ruvia::HttpStatusCode::fromValue(status), std::to_string(code), message);
}

inline OperationResponse operation(ruvia::Context& c, std::string_view message) {
    OperationResponse response(c);
    response.set<"code">(0).set<"message">(message);
    return response;
}

template <typename ResponseT, typename DataT> inline ResponseT ok(ruvia::Context& c, DataT&& data) {
    ResponseT response(c);
    response.set<"code">(0).set<"message">("ok").set<"data">(std::forward<DataT>(data));
    return response;
}

inline HealthResponse health(ruvia::Context& c) {
    HealthData data(c);
    data.set<"status">("ok");
    HealthResponse response(c);
    response.set<"code">(0).set<"message">("ok").set<"data">(std::move(data));
    return response;
}

inline CountResponse count(ruvia::Context& c, std::int64_t createdCount, std::string_view message) {
    CountData data(c);
    data.set<"createdCount">(static_cast<ruvia::Int64>(createdCount));
    CountResponse response(c);
    response.set<"code">(0).set<"message">(message).set<"data">(std::move(data));
    return response;
}

inline ErrorResponse error(ruvia::Context& c, std::int64_t code, std::string_view message) {
    ErrorResponse response(c);
    response.set<"code">(code).set<"message">(message);
    return response;
}

} // namespace service::common
