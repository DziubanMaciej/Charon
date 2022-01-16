#pragma once

#include "charon/user_interface/daemon_user_interface.h"

class DaemonUserInterfaceLinux : public DaemonUserInterface {
public:
    DaemonUserInterfaceLinux(Charon &charon);
    void runImpl() override;
    void kill() override;

private:
    static void sigintHandler(int dummy);
};
