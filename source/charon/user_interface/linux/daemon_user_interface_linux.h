#pragma once

#include "charon/user_interface/daemon_user_interface.h"

class DaemonUserInterfaceLinux : public DaemonUserInterface {
public:
    DaemonUserInterfaceLinux(Charon &charon);
    void run() override;
};
