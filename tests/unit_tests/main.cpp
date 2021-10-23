#include "charon/util/logger.h"

#include <gtest/gtest.h>

int main(int argc, char *argv[]) {
    NullLogger logger{};
    auto loggerSetup = logger.raiiSetup();

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
