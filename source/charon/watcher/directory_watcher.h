#pragma once

#include "charon/watcher/file_event.h"

#include <thread>

class DirectoryWatcher {
public:
    DirectoryWatcher(const std::filesystem::path &directoryPath, FileEventQueue &outputQueue);
    virtual ~DirectoryWatcher() {}

    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual bool isWorking() const = 0;

    const auto &getWatchedDirectory() const { return directoryPath; }

protected:
    const std::filesystem::path directoryPath;
    FileEventQueue &outputQueue;
};
