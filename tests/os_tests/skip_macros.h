#pragma once

#define REQUIRE_FILE_LOCKING_OR_SKIP(filesystem) \
    if (!filesystem.isFileLockingSupported()) {  \
        GTEST_SKIP();                            \
    }
