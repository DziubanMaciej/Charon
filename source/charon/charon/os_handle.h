#pragma once

#if defined(WIN32)
#include <Windows.h>
using OsHandle = HANDLE;
const static inline OsHandle defaultOsHandle = INVALID_HANDLE_VALUE;

#elif linux
using OsHandle = int;
const static inline OsHandle defaultOsHandle = -1;

#else
static_assert(false, "Unsupported system");
#endif
