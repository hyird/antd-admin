#pragma once

#include <cyra/http/Controller.h>

#include "service/modules/system/auth/auth.types.h"

namespace service::modules::system::auth {

class LoginValidator final : public cyra::Middleware<LoginValidator> {
    CYRA_VALIDATE_JSON(LoginBody)
};

class RefreshValidator final : public cyra::Middleware<RefreshValidator> {
    CYRA_VALIDATE_JSON(RefreshBody)
};

}  // namespace service::modules::system::auth
