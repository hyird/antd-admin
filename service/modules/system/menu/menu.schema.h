#pragma once

#include <cyra/http/Controller.h>

#include "service/modules/system/menu/menu.types.h"

namespace service::modules::system::menu {

class CreateMenuValidator final : public cyra::Middleware<CreateMenuValidator> {
    CYRA_VALIDATE_JSON(CreateMenuBody)
};

class UpdateMenuValidator final : public cyra::Middleware<UpdateMenuValidator> {
    CYRA_VALIDATE_JSON(UpdateMenuBody)
};

class ReorderMenuValidator final : public cyra::Middleware<ReorderMenuValidator> {
    CYRA_VALIDATE_JSON(ReorderMenuBody)
};

class BatchCreateMenuButtonsValidator final
    : public cyra::Middleware<BatchCreateMenuButtonsValidator> {
    CYRA_VALIDATE_JSON(BatchCreateMenuButtonsBody)
};

}  // namespace service::modules::system::menu
