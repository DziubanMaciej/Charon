#include "charon/util/time.h"

#include <iomanip>

void TimeImpl::writeCurrentTime(std::ostream &out) const {
    auto time = std::time(nullptr);
    auto localTime = *std::localtime(&time);
    out << std::put_time(&localTime, "%d-%m-%Y %H:%M:%S");
}
