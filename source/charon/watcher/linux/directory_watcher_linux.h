#pragma once

#include "charon/watcher/directory_watcher.h"

class DirectoryWatcherLinux : public DirectoryWatcher {
public:
    DirectoryWatcherLinux(const std::filesystem::path &directoryPath, FileEventQueue &outputQueue, FileEventQueue &deferredOutputQueue);
    ~DirectoryWatcherLinux() override;

    bool start() override;
    bool stop() override;
    bool isWorking() const override;
};
