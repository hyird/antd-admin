#pragma once

#include <cyra/http/Controller.h>

#include "service/modules/system/dept/dept.types.h"

namespace service::modules::system::dept {

class CreateDeptValidator final : public cyra::Middleware<CreateDeptValidator> {
    CYRA_VALIDATE_JSON(CreateDeptBody)
};

class UpdateDeptValidator final : public cyra::Middleware<UpdateDeptValidator> {
    CYRA_VALIDATE_JSON(UpdateDeptBody)
};

}  // namespace service::modules::system::dept
