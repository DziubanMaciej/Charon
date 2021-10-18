#pragma once

#include "charon/processor/deferred_file_locker.h"
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

    void setLogFilePath(const fs::path &path) { logFilePath = path; }
    void setConfigFilePath(const fs::path &path) { configFilePath = path; }
    auto &getLogFilePath() const { return logFilePath; }
    auto &getConfigFilePath() const { return configFilePath; }

    void readUserConsoleInput();

private:
    // Components
    Processor processor;
    DeferredFileLocker deferredFileLocker;
    std::vector<std::unique_ptr<DirectoryWatcher>> directoryWatchers{};

    // Saved file paths
    fs::path logFilePath;
    fs::path configFilePath;

    // Threads for running the components
    std::unique_ptr<std::thread> processorThread{};
    std::unique_ptr<std::thread> deferredFileLockerThread{};

    // Queues for communication between components
    FileEventQueue processorEventQueue{};
    FileEventQueue deferredFileLockerEventQueue{};

    // Basic data
    Logger &logger;
    std::atomic_bool isStarted = false;
};
