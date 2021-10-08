#include "charon/processor/processor.h"
#include "charon/processor/processor_config_reader.h"
#include "charon/util/error.h"
#include "charon/util/filesystem_impl.h"
#include "charon/util/logger.h"
#include "charon/watcher/directory_watcher.h"

#include <iostream>

int main(int argc, char **argv) {
    auto testJson = R"(
[
    {
        "watchedFolder": "D:/Desktop/Test/Src",
        "extensions": [  ],
        "actions": [
            {
                "type": "print"
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
            std::cerr << error << '\n';
        }
        FATAL_ERROR("Error parsing json");
    }

    FileEventQueue eventQueue{};

    auto watcher = DirectoryWatcher::create(config.matchers[0].watchedFolder, eventQueue);
    watcher->start();

    FilesystemImpl filesystem{};
    ConsoleLogger logger{};
    Processor processor{config, eventQueue, filesystem, &logger};

    std::cout << "Running processor...\n";
    processor.run();
    std::cout << "End\n";
}
