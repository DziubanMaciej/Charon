#pragma once

#include "charon/watcher/file_event.h"

#include <thread>

class DirectoryWatcher {
public:
    DirectoryWatcher(const std::filesystem::path &directoryPath, FileEventQueue &outputQueue);
    virtual ~DirectoryWatcher() {}

    static std::unique_ptr<DirectoryWatcher> create(const std::filesystem::path &directoryPath, FileEventQueue &outputQueue);

    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual bool isWorking() const = 0;

protected:
    const std::filesystem::path directoryPath;
    FileEventQueue &outputQueue;
};
