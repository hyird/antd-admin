#pragma once

#include <cyra/http/Controller.h>

#include "service/modules/system/role/role.types.h"

namespace service::modules::system::role {

class CreateRoleValidator final : public cyra::Middleware<CreateRoleValidator> {
    CYRA_VALIDATE_JSON(CreateRoleBody)
};

class UpdateRoleValidator final : public cyra::Middleware<UpdateRoleValidator> {
    CYRA_VALIDATE_JSON(UpdateRoleBody)
};

}  // namespace service::modules::system::role
