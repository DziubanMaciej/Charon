#include "charon/util/logger.h"
#include "charon/util/time.h"
#include "os_tests/test_files_helper.h"
#include "os_tests/test_helpers.h"

#include <regex>

void populateCaches() {
    {
        std::filesystem::path path = TestFilesHelper::createDirectory("src");
        TestFilesHelper::createFile(path / "abc");
    }
    {
        const std::regex base_regex("abc");
    }
    {
        const auto path = TestFilesHelper::getTestFilePath("log.txt");
        TimeImpl time{};
        FileLogger logger{time, path, defaultLogLevel};
        logger.log(LogLevel::Error, "abc");
    }
    {
        TimeImpl time{};
        ConsoleLogger logger{time, defaultLogLevel};
        logger.log(LogLevel::Info, "Caches populated");
    }
}
