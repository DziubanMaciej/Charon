#include "charon/util/logger.h"
#include "common/oakum_gtest_event_listener.h"

#include <gtest/gtest.h>

int main(int argc, char *argv[]) {
    NullLogger logger{};
    auto loggerSetup = logger.raiiSetup();

    // TODO there are problems with gmock and oakum in unit tests
    // testing::TestEventListeners &listeners = testing::UnitTest::GetInstance()->listeners();
    // listeners.Append(new OakumGtestEventListener(true));

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
