#pragma once

#include "charon/charon/os_handle.h"

#if defined(WIN32)
#define WIDEN(x) L##x
#define CSTRING(x) WIDEN(x)

#elif linux
#define CSTRING(x) x

#else
static_assert(false, "Unsupported system");
#endif
