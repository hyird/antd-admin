#pragma once

#include <charconv>
#include <chrono>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include <ruvia/web/App.h>
#include <ruvia/web/auth/Jwt.h>

namespace service::core {

struct JwtPayload {
    std::int64_t user_id{0};
    std::string username;
    std::int64_t iat{0};
    std::int64_t exp{0};
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
    if (!secret || secret->empty()) {
        throw std::runtime_error("JWT_SECRET environment variable is required");
    }
    return std::string(*secret);
}

inline std::string refreshSecret() {
    auto secret = ruvia::app().env().get("JWT_REFRESH_SECRET");
    if (!secret) {
        return accessSecret();
    }
    if (secret->empty()) {
        throw std::runtime_error("JWT_REFRESH_SECRET must not be empty");
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
    ruvia::JwtSignOptions options;
    options.secret.assign(secret.data(), secret.size());
    const std::string subject = std::to_string(payload.user_id);
    options.subject.assign(subject.data(), subject.size());
    options.expiresIn = expiresIn;

    // JwtClaim now owns pmr strings (copied from the given views); no field setters.
    options.claims.emplace_back("user_id", subject);
    options.claims.emplace_back("username", payload.username);

    const auto token = ruvia::jwtSign(options);
    return std::string(token.data(), token.size());
}

inline service::core::JwtPayload verify(const std::string& token, const std::string& secret) {
    try {
        ruvia::JwtVerifyOptions options;
        options.secret.assign(secret.data(), secret.size());
        const auto payload = ruvia::jwtVerify(token, options);

        service::core::JwtPayload out;
        if (auto userId = payload.claim("user_id")) {
            out.user_id = std::stoll(std::string(*userId));
        }
        if (auto username = payload.claim("username")) {
            out.username = std::string(*username);
        }
        if (payload.issuedAt()) {
            out.iat = std::chrono::duration_cast<std::chrono::seconds>(
                          payload.issuedAt()->time_since_epoch())
                          .count();
        }
        if (payload.expiresAt()) {
            out.exp = std::chrono::duration_cast<std::chrono::seconds>(
                          payload.expiresAt()->time_since_epoch())
                          .count();
        }
        if (out.user_id <= 0 || out.username.empty()) {
            throw utils::JwtInvalidError("invalid token payload");
        }
        return out;
    } catch (const utils::JwtInvalidError&) {
        throw;
    } catch (const std::exception& ex) {
        const std::string message(ex.what());
        if (message.find("expired") != std::string::npos) {
            throw utils::JwtExpiredError(message);
        }
        throw utils::JwtInvalidError(message);
    }
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
