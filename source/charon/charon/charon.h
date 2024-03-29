#pragma once

#include "charon/processor/deferred_file_locker.h"
#include "charon/processor/processor.h"
#include "charon/util/class_traits.h"
#include "charon/util/filesystem.h"
#include "charon/util/notification.h"
#include "charon/watcher/directory_watcher.h"

#include <vector>

struct DirectoryWatcherFactory;

class Charon : NonCopyableAndMovable {
public:
    Charon(const ProcessorConfig &config, Filesystem &filesystem, DirectoryWatcherFactory &watcherFactory);

    bool start();
    bool stop();
    void waitForCompletion();

    void processImmediate(const std::vector<fs::path> &paths);

    void setLogFilePath(const fs::path &path) { logFilePath = path; }
    void setConfigFilePath(const fs::path &path) { configFilePath = path; }
    auto &getLogFilePath() const { return logFilePath; }
    auto &getConfigFilePath() const { return configFilePath; }

private:
    // Components
    DeferredFileLocker deferredFileLocker;
    Processor processor;
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
    Notification isStarted{};
};
