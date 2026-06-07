#pragma once

#include <cyra/http/Controller.h>

#include "service/modules/system/user/user.types.h"

namespace service::modules::system::user {

class CreateUserValidator final : public cyra::Middleware<CreateUserValidator> {
    CYRA_VALIDATE_JSON(CreateUserBody)
};

class UpdateUserValidator final : public cyra::Middleware<UpdateUserValidator> {
    CYRA_VALIDATE_JSON(UpdateUserBody)
};

}  // namespace service::modules::system::user
