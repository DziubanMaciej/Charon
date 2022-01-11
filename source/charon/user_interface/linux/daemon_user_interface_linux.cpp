
#include "charon/user_interface/linux/daemon_user_interface_linux.h"
#include "charon/util/error.h"
#include "charon/util/logger.h"

#include <signal.h>

std::unique_ptr<DaemonUserInterface> DaemonUserInterface::create(Charon &charon) {
    return std::unique_ptr<DaemonUserInterface>(new DaemonUserInterfaceLinux(charon));
}

DaemonUserInterfaceLinux::DaemonUserInterfaceLinux(Charon &charon)
    : DaemonUserInterface(charon) {}

void DaemonUserInterfaceLinux::runImpl() {
    struct sigaction act;
    act.sa_handler = DaemonUserInterfaceLinux::sigintHandler;
    sigaction(SIGINT, &act, NULL);

    charon.waitForCompletion();
}

void DaemonUserInterfaceLinux::kill() {
    charon.stop();
}

void DaemonUserInterfaceLinux::sigintHandler([[maybe_unused]] int dummy) {
    log(LogLevel::Info) << "User interruption (^C) detected. Exiting.";
    instance->kill();
}
