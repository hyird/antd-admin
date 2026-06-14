#pragma once

#include <charconv>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

#include <ruvia/http/Context.h>
#include <ruvia/http/Error.h>
#include <ruvia/http/HttpTypes.h>

#include "service/common/types.h"

namespace service::common {

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

RUVIA_MODEL(ErrorResponse, RUVIA_FIELD(code, ruvia::Int64), RUVIA_FIELD(message, ruvia::String));

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
    throw ruvia::HttpError(def.status, std::to_string(def.code), def.message);
}

[[noreturn]] inline void throwAppError(std::int64_t code, std::string message,
                                       std::uint16_t status = 400) {
    throw ruvia::HttpError(status, std::to_string(code), message);
}

inline OperationResponse operation(ruvia::Context& c, std::string_view message) {
    OperationResponse response(c);
    response.code(0).message(message);
    return response;
}

template <typename ResponseT, typename DataT> inline ResponseT ok(ruvia::Context& c, DataT&& data) {
    ResponseT response(c);
    response.code(0).message("ok").data(std::forward<DataT>(data));
    return response;
}

inline HealthResponse health(ruvia::Context& c) {
    HealthData data(c);
    data.status("ok");
    HealthResponse response(c);
    response.code(0).message("ok").data(std::move(data));
    return response;
}

inline CountResponse count(ruvia::Context& c, std::int64_t createdCount, std::string_view message) {
    CountData data(c);
    data.createdCount(static_cast<ruvia::Int64>(createdCount));
    CountResponse response(c);
    response.code(0).message(message).data(std::move(data));
    return response;
}

inline ErrorResponse error(ruvia::Context& c, std::int64_t code, std::string_view message) {
    ErrorResponse response(c);
    response.code(code).message(message);
    return response;
}

} // namespace service::common
