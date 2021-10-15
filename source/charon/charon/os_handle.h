#pragma once

#if defined(WIN32)
#include <Windows.h>
using OsHandle = HANDLE;
#else
static_assert(false, "Unsupported system");
#endif
