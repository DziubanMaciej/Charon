#include "charon/util/logger.h"
#include "unit_tests/mocks/mock_logger.h"
#include "unit_tests/mocks/mock_time.h"

#include <gtest/gtest.h>

using ::testing::Ref;

TEST(LoggerTest, givenVariousLogLevelsWhenRaiiLogIsCalledThenPrependTheLogWithLogLevelString) {
    MockLogger logger{};
    EXPECT_CALL(logger, log(LogLevel::Error, "a 1"));
    EXPECT_CALL(logger, log(LogLevel::Info, "b 1"));
    EXPECT_CALL(logger, log(LogLevel::Warning, "c 1"));
    EXPECT_CALL(logger, log(LogLevel::Debug, "d 1"));

    log(LogLevel::Error, &logger) << "a " << 1;
    log(LogLevel::Info, &logger) << "b " << 1;
    log(LogLevel::Warning, &logger) << "c " << 1;
    log(LogLevel::Debug, &logger) << "d " << 1;
}

TEST(LoggerTest, givenLoggerIsSetUpWhenLogFunctionIsCalledThenUsedTheLogger) {
    MockLogger logger1{};
    MockLogger logger2{};

    EXPECT_CALL(logger1, log(LogLevel::Error, "1"));
    EXPECT_CALL(logger2, log(LogLevel::Error, "2"));
    EXPECT_CALL(logger1, log(LogLevel::Error, "3"));

    auto setup1 = logger1.raiiSetup();
    log(LogLevel::Error) << "1";
    {
        auto setup2 = logger2.raiiSetup();
        log(LogLevel::Error) << "2";
    }
    log(LogLevel::Error) << "3";
}

TEST(LoggerTest, givenNoLoggerIsPassedOrGloballySetupWhenLoggingThenThrowError) {
    Logger::RaiiSetup setup{nullptr};
    EXPECT_ANY_THROW((log(LogLevel::Error) << "3"));
}

TEST(LoggerTest, whenConsoleLoggerIsUsedThenPrintMessagesToConsole) {
    MockTime time{};
    EXPECT_CALL(time, writeCurrentTime(Ref(std::cout)))
        .Times(2)
        .WillOnce([](std::ostream &out) { out << "date1"; })
        .WillOnce([](std::ostream &out) { out << "date2"; });

    ConsoleLogger logger{time};

    ::testing::internal::CaptureStdout();
    logger.log(LogLevel::Info, "Hello world");
    EXPECT_STREQ("[date1][Info] Hello world\n", testing::internal::GetCapturedStdout().c_str());

    ::testing::internal::CaptureStdout();
    log(LogLevel::Info, &logger) << "Hello world";
    EXPECT_STREQ("[date2][Info] Hello world\n", testing::internal::GetCapturedStdout().c_str());
}

TEST(LoggerTest, whenNullLoggerIsUsedThenDoNotPrintAnything) {
    NullLogger logger{};

    ::testing::internal::CaptureStdout();
    logger.log(LogLevel::Info, "Hello world");
    EXPECT_TRUE(testing::internal::GetCapturedStdout().empty());

    ::testing::internal::CaptureStdout();
    log(LogLevel::Info, &logger) << "Hello world";
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
    log(LogLevel::Error, &logger) << "message";
}
