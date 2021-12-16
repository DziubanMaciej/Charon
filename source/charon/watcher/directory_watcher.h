#pragma once

#include "charon/util/class_traits.h"
#include "charon/watcher/file_event.h"

#include <thread>

class DirectoryWatcher : NonCopyableAndMovable {
public:
    DirectoryWatcher(const std::filesystem::path &directoryPath, FileEventQueue &outputQueue, FileEventQueue &deferredOutputQueue);
    virtual ~DirectoryWatcher() {}

    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual bool isWorking() const = 0;

    const auto &getWatchedDirectory() const { return directoryPath; }

protected:
    void pushEvent(FileEvent &&fileEvent);

    const std::filesystem::path directoryPath;

private:
    FileEventQueue &outputQueue;
    FileEventQueue &deferredOutputQueue;
};
