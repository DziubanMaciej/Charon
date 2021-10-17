#pragma once

#include "charon/watcher/directory_watcher.h"

#include <memory>

struct DirectoryWatcherFactory {
    virtual std::unique_ptr<DirectoryWatcher> create(const std::filesystem::path &directoryPath,
                                                     FileEventQueue &outputQueue,
                                                     FileEventQueue &deferredOutputQueue) = 0;
};

struct DirectoryWatcherFactoryImpl : DirectoryWatcherFactory {
    std::unique_ptr<DirectoryWatcher> create(const std::filesystem::path &directoryPath,
                                             FileEventQueue &outputQueue,
                                             FileEventQueue &deferredOutputQueue) override;
};
