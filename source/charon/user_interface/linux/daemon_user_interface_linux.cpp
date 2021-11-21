
#include "charon/user_interface/daemon_user_interface.h"
#include "charon/util/error.h"

std::unique_ptr<DaemonUserInterface> DaemonUserInterface::create([[maybe_unused]] Charon &charon) {
    FATAL_ERROR("DaemonUserInterface is not supported on Linux");
}
