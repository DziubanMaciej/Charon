#pragma once

#include "charon/processor/processor.h"
#include "charon/util/class_traits.h"
#include "charon/util/filesystem.h"
#include "charon/watcher/directory_watcher.h"

#include <vector>

class Logger;
class DirectoryWatcherFactory;

class Charon : NonCopyableAndMovable {
public:
    Charon(const ProcessorConfig &config, Filesystem &filesystem, Logger &logger, DirectoryWatcherFactory &watcherFactory);

    bool start();
    bool stop();

    void readUserConsoleInput();

private:
    FileEventQueue eventQueue{};
    Processor processor;
    std::unique_ptr<std::thread> processorThread{};
    std::vector<std::unique_ptr<DirectoryWatcher>> directoryWatchers{};
    Logger &logger;
    std::atomic_bool isStarted = false;
};
