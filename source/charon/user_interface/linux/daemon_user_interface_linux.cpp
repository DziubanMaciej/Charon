
#include "charon/user_interface/linux/daemon_user_interface_linux.h"
#include "charon/util/error.h"

std::unique_ptr<DaemonUserInterface> DaemonUserInterface::create(Charon &charon) {
    return std::unique_ptr<DaemonUserInterface>(new DaemonUserInterfaceLinux(charon));
}

DaemonUserInterfaceLinux::DaemonUserInterfaceLinux(Charon &charon)
    : DaemonUserInterface(charon) {}

void DaemonUserInterfaceLinux::run() {
    charon.waitForCompletion();
}
