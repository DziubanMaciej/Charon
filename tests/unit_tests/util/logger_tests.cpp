#include "charon/util/logger.h"
#include "unit_tests/mocks/mock_logger.h"

#include <gtest/gtest.h>

TEST(LoggerTest, givenVariousLogLevelsWhenRaiiLogIsCalledThenPrependTheLogWithLogLevelString) {
    MockLogger logger{};
    EXPECT_CALL(logger, log(LogLevel::Error, "a 1"));
    EXPECT_CALL(logger, log(LogLevel::Info, "b 1"));
    EXPECT_CALL(logger, log(LogLevel::Warning, "c 1"));
    EXPECT_CALL(logger, log(LogLevel::Debug, "d 1"));

    log(logger, LogLevel::Error) << "a " << 1;
    log(logger, LogLevel::Info) << "b " << 1;
    log(logger, LogLevel::Warning) << "c " << 1;
    log(logger, LogLevel::Debug) << "d " << 1;
}

TEST(LoggerTest, whenConsoleLoggerIsUsedThenPrintMessagesToConsole) {
    ConsoleLogger logger{};

    ::testing::internal::CaptureStdout();
    logger.log(LogLevel::Info, "Hello world");
    EXPECT_STREQ("[Info] Hello world\n", testing::internal::GetCapturedStdout().c_str());

    ::testing::internal::CaptureStdout();
    log(logger, LogLevel::Info) << "Hello world";
    EXPECT_STREQ("[Info] Hello world\n", testing::internal::GetCapturedStdout().c_str());
}

TEST(LoggerTest, whenNullLoggerIsUsedThenDoNotPrintAnything) {
    NullLogger logger{};

    ::testing::internal::CaptureStdout();
    logger.log(LogLevel::Info, "Hello world");
    EXPECT_TRUE(testing::internal::GetCapturedStdout().empty());

    ::testing::internal::CaptureStdout();
    log(logger, LogLevel::Info) << "Hello world";
    EXPECT_TRUE(testing::internal::GetCapturedStdout().empty());
}

TEST(LoggerTest, whenMultiplexedLoggerIsUsedThenCallAllLoggers) {
    MockLogger logger1{};
    MockLogger logger2{};
    MockLogger logger3{};

    EXPECT_CALL(logger1, log(LogLevel::Error, "message"));
    EXPECT_CALL(logger2, log(LogLevel::Error, "message"));
    EXPECT_CALL(logger3, log(LogLevel::Error, "message"));

    MultiplexedLogger logger{&logger1, &logger2, &logger3};
    log(logger, LogLevel::Error) << "message";
}
