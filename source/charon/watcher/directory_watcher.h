#pragma once

#include "charon/util/class_traits.h"
#include "charon/watcher/file_event.h"

#include <thread>

class DirectoryWatcher : NonCopyableAndMovable {
public:
    DirectoryWatcher(const std::filesystem::path &directoryPath, FileEventQueue &outputQueue, FileEventQueue &deferredOutputQueue);
    virtual ~DirectoryWatcher() {}

    bool start();
    bool stop();
    virtual bool isWorking() const = 0;

    const auto &getWatchedDirectory() const { return directoryPath; }

protected:
    virtual bool startImpl() = 0;
    virtual bool stopImpl() = 0;
    void pushEvent(FileEvent &&fileEvent);

    const std::filesystem::path directoryPath;

private:
    FileEventQueue &outputQueue;
    FileEventQueue &deferredOutputQueue;
};
