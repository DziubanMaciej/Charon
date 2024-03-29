#pragma once

#include "oakum/oakum_api.h"

// This macro calls undocumented, internal googletest macro, because GTEST_SKIP() allocates memory internally
// and then returns from test. We have to split these to actions to tell Oakum to  ignore these internal memory
// allocations
#define SKIP()                                                                      \
    do {                                                                            \
        oakumStartIgnore(); /* don't care about errors, nothing bad should happen*/ \
        GTEST_MESSAGE_("", ::testing::TestPartResult::kSkip);                       \
        oakumStopIgnore();                                                          \
        return;                                                                     \
    } while (false)

#define REQUIRE_FILE_LOCKING_OR_SKIP(filesystem) \
    if (!filesystem.isFileLockingSupported()) {  \
        SKIP();                                  \
    }

// In release config some compilers may generate some lazily populated caches for STD classes. These can be reported
// as memory leaks. We can call them early, to populate these caches before any tests.
void populateCaches();
