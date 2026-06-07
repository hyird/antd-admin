#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

#include <cyra/http/Context.h>
#include <cyra/http/Error.h>
#include <cyra/http/HttpTypes.h>

#include "service/common/types.h"

namespace service::common {

struct AppErrorDef {
    std::string_view code;
    std::string_view message;
    std::uint16_t status{400};
};

CYRA_MODEL(ErrorResponse,
    CYRA_FIELD(code, cyra::String),
    CYRA_FIELD(message, cyra::String)
);

[[noreturn]] inline void throwAppError(const AppErrorDef& def) {
    throw cyra::HttpError(def.status, def.code, def.message);
}

[[noreturn]] inline void throwAppError(std::string code,
                                       std::string message,
                                       std::uint16_t status = 400) {
    throw cyra::HttpError(status, code, message);
}

inline OperationResponse operation(cyra::Context& c, std::string_view message) {
    OperationResponse response(c);
    response.code(0).message(message);
    return response;
}

template <typename ResponseT, typename DataT>
inline ResponseT ok(cyra::Context& c, DataT&& data) {
    ResponseT response(c);
    response.code(0).message("ok").data(std::forward<DataT>(data));
    return response;
}

inline HealthResponse health(cyra::Context& c) {
    HealthData data(c);
    data.status("ok");
    HealthResponse response(c);
    response.code(0).message("ok").data(std::move(data));
    return response;
}

inline CountResponse count(cyra::Context& c, std::int64_t createdCount, std::string_view message) {
    CountData data(c);
    data.createdCount(static_cast<cyra::Int64>(createdCount));
    CountResponse response(c);
    response.code(0).message(message).data(std::move(data));
    return response;
}

inline ErrorResponse error(cyra::Context& c, std::string_view code, std::string_view message) {
    ErrorResponse response(c);
    response.code(code).message(message);
    return response;
}

}  // namespace service::common
