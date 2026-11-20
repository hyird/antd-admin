#pragma once

#include <array>
#include <charconv>
#include <chrono>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include <app.h>
#include <jwt.h>

namespace service::core {

struct JwtPayload {
    std::int64_t user_id{0};
    std::string username;
};

} // namespace service::core

namespace service::utils {

class JwtExpiredError : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};

class JwtInvalidError : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};

} // namespace service::utils

namespace service::jwt_detail {

inline std::string accessSecret() {
    auto secret = ruvia::app().env().get("JWT_SECRET");
    if (!secret || secret->size() < 32) {
        throw std::runtime_error("JWT_SECRET must contain at least 32 bytes");
    }
    return std::string(*secret);
}

inline std::string refreshSecret() {
    auto secret = ruvia::app().env().get("JWT_REFRESH_SECRET");
    if (!secret) {
        return accessSecret();
    }
    if (secret->size() < 32) {
        throw std::runtime_error("JWT_REFRESH_SECRET must contain at least 32 bytes");
    }
    return std::string(*secret);
}

inline std::chrono::seconds parseDuration(std::string_view value, std::string_view configName) {
    if (value.empty()) {
        throw std::runtime_error(std::string(configName) + " must not be empty");
    }

    std::string_view number = value;
    std::int64_t multiplier = 1;
    const char suffix = value.back();
    if (suffix < '0' || suffix > '9') {
        number.remove_suffix(1);
        switch (suffix) {
        case 's':
            multiplier = 1;
            break;
        case 'm':
            multiplier = 60;
            break;
        case 'h':
            multiplier = 60 * 60;
            break;
        case 'd':
            multiplier = 60 * 60 * 24;
            break;
        default:
            throw std::runtime_error(std::string(configName) + " must use an s, m, h, or d suffix");
        }
    }

    std::int64_t count = 0;
    const auto [end, error] = std::from_chars(number.data(), number.data() + number.size(), count);
    if (error != std::errc{} || end != number.data() + number.size() || count <= 0 ||
        count > std::numeric_limits<std::int64_t>::max() / multiplier) {
        throw std::runtime_error(std::string(configName) + " must be a positive duration");
    }
    return std::chrono::seconds(count * multiplier);
}

inline std::chrono::seconds accessExpiresIn() {
    return parseDuration(ruvia::app().env().get("JWT_EXPIRES_IN").value_or("1d"), "JWT_EXPIRES_IN");
}

inline std::chrono::seconds refreshExpiresIn() {
    return parseDuration(ruvia::app().env().get("JWT_REFRESH_EXPIRES_IN").value_or("7d"),
                         "JWT_REFRESH_EXPIRES_IN");
}

inline std::string sign(const service::core::JwtPayload& payload, const std::string& secret,
                        std::chrono::seconds expiresIn) {
    const auto subject = std::to_string(payload.user_id);
    const std::array claims{ruvia::jwt_claim_view{"user_id", subject},
                            ruvia::jwt_claim_view{"username", payload.username}};
    return ruvia::sign_jwt(
        {.secret_ = secret, .subject_ = subject, .expires_in_ = expiresIn, .claims_ = claims});
}

inline service::core::JwtPayload verify(const std::string& token, const std::string& secret) {
    const auto result = ruvia::verify_jwt({.token_ = token, .secret_ = secret});
    if (result.error() == ruvia::jwt_error_code::expired)
        throw utils::JwtExpiredError("token expired");
    if (!result.has_value())
        throw utils::JwtInvalidError("invalid token");

    const auto& payload = result.value();
    const auto userId = payload.claim("user_id").value_or("");
    const auto username = payload.claim("username").value_or("");
    std::int64_t id = 0;
    const auto [end, error] = std::from_chars(userId.data(), userId.data() + userId.size(), id);
    if (error != std::errc{} || end != userId.data() + userId.size() || id <= 0 || username.empty())
        throw utils::JwtInvalidError("invalid token payload");
    return {id, std::string(username)};
}

} // namespace service::jwt_detail

namespace service::utils {

inline void validateJwtConfiguration() {
    (void)jwt_detail::accessSecret();
    (void)jwt_detail::refreshSecret();
    (void)jwt_detail::accessExpiresIn();
    (void)jwt_detail::refreshExpiresIn();
}

inline std::string signAccessToken(const service::core::JwtPayload& payload) {
    return jwt_detail::sign(payload, jwt_detail::accessSecret(), jwt_detail::accessExpiresIn());
}

inline std::string signRefreshToken(const service::core::JwtPayload& payload) {
    return jwt_detail::sign(payload, jwt_detail::refreshSecret(), jwt_detail::refreshExpiresIn());
}

inline service::core::JwtPayload verifyAccessToken(const std::string& token) {
    return jwt_detail::verify(token, jwt_detail::accessSecret());
}

inline service::core::JwtPayload verifyRefreshToken(const std::string& token) {
    return jwt_detail::verify(token, jwt_detail::refreshSecret());
}

} // namespace service::utils
