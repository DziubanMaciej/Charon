#include "charon/processor/processor_config_reader.h"
#include "charon/watcher/directory_watcher.h"

#include <iostream>

int main(int argc, char **argv) {
    auto testJson = R"(
[
    {
        "watchedFolder": "D:/Desktop/Test/Src",
        "extensions": [ "png", "jpg", "gif" ],
        "actions": [
            {
                "type": "copy",
                "destinationDir": "D:/Desktop/Dst1",
                "destinationName": "###.$ext"
            },
            {
                "type": "move",
                "destinationDir": "D:/Desktop/Dst2",
                "destinationName": "###.$ext"
            }
        ]
    }
]
)";
    ProcessConfigReader reader{};
    ProcessorConfig config{};
    bool success = reader.read(config, std::string{testJson});

    BlockingQueue<FileAction> actions{};
    auto watcher1 = DirectoryWatcher::create(argv[1], actions);
    auto watcher2 = DirectoryWatcher::create(argv[1], actions);

    bool a = watcher1->isWorking();
    bool b = watcher2->isWorking();

    watcher1->start();
    watcher2->start();

    watcher1->stop();
    watcher1->start();

    std::cout << "Actions:\n";
    while (true) {
        FileAction action{};
        if (!actions.blockingPop(action)) {
            std::cout << "  ERROR: could not pop\n";
        } else {
            std::cout << "  File event: " << action.watchedRootPath << ", " << action.path << '\n';
        }
    }
}
