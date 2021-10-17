#pragma once

#include "charon/charon/os_handle.h"

#if defined(WIN32)
const static inline OsHandle mockOsHandle = reinterpret_cast<HANDLE>(0x1234);
#else
static_assert(false, "Unsupported system");
#endif
