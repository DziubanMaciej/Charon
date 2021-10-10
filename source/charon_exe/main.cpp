#include "charon/charon/charon.h"
#include "charon/processor/processor_config_reader.h"
#include "charon/util/filesystem_impl.h"
#include "charon/util/logger.h"
#include "charon/watcher/directory_watcher_factory.h"

#include <iostream>

int main(int argc, char **argv) {
    ConsoleLogger logger{};

    auto testJson = R"(
[
    {
        "watchedFolder": "D:/Desktop/Test/Src",
        "extensions": [ "jpg" ],
        "actions": [
            {
                "type": "print"
            },
            {
                "type": "copy",
                "destinationDir": "D:/Desktop/Test/Dst",
                "destinationName": "file_###"
            }
        ]
    }
]
)";
    ProcessConfigReader reader{};
    ProcessorConfig config{};
    const bool success = reader.read(config, std::string{testJson});
    if (!success) {
        for (auto &error : reader.getErrors()) {
            log(logger, LogLevel::Error) << "json parsing error: " << error;
        }
        return EXIT_FAILURE;
    }

    FilesystemImpl filesystem{};
    DirectoryWatcherFactoryImpl watcherFactory{};

    Charon charon{config, filesystem, logger, watcherFactory};
    if (!charon.runWatchers()) {
        return EXIT_FAILURE;
    }

    std::thread consoleInputThread([&]() { charon.readUserConsoleInput(); });
    charon.runProcessor();
    consoleInputThread.join();
}
