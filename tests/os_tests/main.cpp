#include "os_tests/test_files_helper.h"

#include <gtest/gtest.h>

class CustomGtestEventListener : public ::testing::EmptyTestEventListener {
public:
    void OnTestStart(const ::testing::TestInfo &test_info) override {
        TestFilesHelper::cleanupTestDirectory();
    }
};

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);

    testing::TestEventListeners &listeners = testing::UnitTest::GetInstance()->listeners();
    listeners.Append(new CustomGtestEventListener());

    return RUN_ALL_TESTS();
}
