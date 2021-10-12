#include "charon/charon/charon.h"
#include "charon/processor/processor_config_reader.h"
#include "charon/util/filesystem_impl.h"
#include "charon/util/logger.h"
#include "charon/watcher/directory_watcher_factory.h"

#include <iostream>

int main(int argc, char **argv) {
    FileLogger fileLogger{fs::current_path() / "log.txt"};
    ConsoleLogger consoleLogger{};
    MultiplexedLogger logger{&fileLogger, &consoleLogger};

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
    ProcessConfigReader reader{logger};
    ProcessorConfig config{};
    const bool success = reader.read(config, std::string{testJson});
    if (!success) {
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
