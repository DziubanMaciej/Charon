#include "charon/util/logger.h"
#include "os_tests/test_files_helper.h"

#include <gtest/gtest.h>

struct TestDirectoryEventListener : ::testing::EmptyTestEventListener {
public:
    void OnTestStart(const ::testing::TestInfo &testInfo) override {
        TestFilesHelper::cleanupTestDirectory();
    }

    void OnTestEnd(const ::testing::TestInfo &testInfo) override {
        TestFilesHelper::cleanupTestDirectory();
    }
};

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);

    NullLogger logger{};
    auto loggerSetup = logger.raiiSetup();

    testing::TestEventListeners &listeners = testing::UnitTest::GetInstance()->listeners();
    listeners.Append(new TestDirectoryEventListener());

    return RUN_ALL_TESTS();
}
