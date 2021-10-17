#pragma once

#include "charon/user_interface/user_interface.h"

struct DaemonUserInterface : UserInterface {
    using UserInterface::UserInterface;

    static std::unique_ptr<DaemonUserInterface> create(Charon &charon);
};
