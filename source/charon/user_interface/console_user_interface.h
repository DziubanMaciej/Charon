#pragma once

#include "charon/user_interface/user_interface.h"

struct ConsoleUserInterface : UserInterface {
    using UserInterface::UserInterface;

    void run() override;
};
