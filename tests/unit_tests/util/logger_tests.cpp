#include "charon/util/logger.h"
#include "unit_tests/mocks/mock_logger.h"

#include <gtest/gtest.h>

TEST(LoggerTest, givenVariousLogLevelsWhenRaiiLogIsCalledThenPrependTheLogWithLogLevelString) {
    MockLogger logger{};
    EXPECT_CALL(logger, log("[Error] a 1\n"));
    EXPECT_CALL(logger, log("[Info] b 1\n"));
    EXPECT_CALL(logger, log("[Warning] c 1\n"));
    EXPECT_CALL(logger, log("[Debug] d 1\n"));

    log(logger, LogLevel::Error) << "a " << 1;
    log(logger, LogLevel::Info) << "b " << 1;
    log(logger, LogLevel::Warning) << "c " << 1;
    log(logger, LogLevel::Debug) << "d " << 1;
}

TEST(LoggerTest, whenConsoleLoggerIsUsedThenPrintMessagesToConsole) {
    ConsoleLogger logger{};

    ::testing::internal::CaptureStdout();
    logger.log("Hello world");
    EXPECT_STREQ("Hello world", testing::internal::GetCapturedStdout().c_str());

    ::testing::internal::CaptureStdout();
    log(logger, LogLevel::Info) << "Hello world";
    EXPECT_STREQ("[Info] Hello world\n", testing::internal::GetCapturedStdout().c_str());
}

TEST(LoggerTest, whenNullLoggerIsUsedThenDoNotPrintAnything) {
    NullLogger logger{};

    ::testing::internal::CaptureStdout();
    logger.log("Hello world");
    EXPECT_TRUE(testing::internal::GetCapturedStdout().empty());

    ::testing::internal::CaptureStdout();
    log(logger, LogLevel::Info) << "Hello world";
    EXPECT_TRUE(testing::internal::GetCapturedStdout().empty());
}
