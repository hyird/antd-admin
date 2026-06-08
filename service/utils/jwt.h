#pragma once

#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include <cyra/app/App.h>
#include <cyra/auth/Jwt.h>

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
    auto secret = cyra::app().env().get("JWT_SECRET");
    if (!secret) {
        throw std::runtime_error("JWT_SECRET environment variable is required");
    }
    return std::string(*secret);
}

inline std::string refreshSecret() {
    auto secret = cyra::app().env().get("JWT_REFRESH_SECRET");
    return secret ? std::string(*secret) : accessSecret();
}

inline std::chrono::seconds parseDuration(std::string_view value, std::chrono::seconds fallback) {
    if (value.empty())
        return fallback;
    const char suffix = value.back();
    std::string number(value);
    std::int64_t multiplier = 1;
    if (suffix == 's' || suffix == 'm' || suffix == 'h' || suffix == 'd') {
        number = value.substr(0, value.size() - 1);
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
            break;
        }
    }
    try {
        return std::chrono::seconds(std::stoll(number) * multiplier);
    } catch (...) {
        return fallback;
    }
}

inline std::chrono::seconds accessExpiresIn() {
    return parseDuration(cyra::app().env().get("JWT_EXPIRES_IN").value_or("1d"),
                         std::chrono::seconds(60 * 60 * 24));
}

inline std::chrono::seconds refreshExpiresIn() {
    return parseDuration(cyra::app().env().get("JWT_REFRESH_EXPIRES_IN").value_or("7d"),
                         std::chrono::seconds(60 * 60 * 24 * 7));
}

inline std::string sign(const service::core::JwtPayload& payload, const std::string& secret,
                        std::chrono::seconds expiresIn) {
    cyra::JwtSignOptions options;
    options.secret = secret;
    options.subject = std::to_string(payload.user_id);
    options.expiresIn = expiresIn;

    cyra::JwtClaim userId;
    userId.name = "user_id";
    userId.value = std::to_string(payload.user_id);
    options.claims.push_back(std::move(userId));

    cyra::JwtClaim username;
    username.name = "username";
    username.value = payload.username;
    options.claims.push_back(std::move(username));

    return std::string(cyra::jwtSign(options));
}

inline service::core::JwtPayload verify(const std::string& token, const std::string& secret) {
    try {
        cyra::JwtVerifyOptions options;
        options.secret = secret;
        const auto payload = cyra::jwtVerify(token, options);

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
