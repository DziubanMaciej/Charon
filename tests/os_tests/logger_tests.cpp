#include "charon/util/logger.h"
#include "charon/util/time.h"
#include "os_tests/test_files_helper.h"

#include <gtest/gtest.h>

TEST(LoggerTest, givenFileLoggerWhenItIsCreatedThenItAppendsToAnExistingFile) {
    const auto path = TestFilesHelper::getTestFilePath("log.txt");
    TimeImpl time{};

    {
        FileLogger logger{time, path};
        logger.log(LogLevel::Error, "Hello");
        logger.log(LogLevel::Error, "Hello");
        logger.log(LogLevel::Error, "Hello");
    }
    EXPECT_EQ(3u, TestFilesHelper::countLinesInFile(path));

    {
        FileLogger logger{time, path};
        logger.log(LogLevel::Error, "Hello");
        logger.log(LogLevel::Error, "Hello");
        logger.log(LogLevel::Error, "Hello");
    }
    EXPECT_EQ(6u, TestFilesHelper::countLinesInFile(path));
}
