#pragma once

#include "charon/util/error.h"

#include <cstring>

#define FATAL_ERROR_IF_SYSCALL_FAILED(ret, ...)                                  \
    if (ret < 0) {                                                               \
        FATAL_ERROR(__VA_ARGS__, " (errno=", errno, ": ", strerror(errno), ")"); \
    }
