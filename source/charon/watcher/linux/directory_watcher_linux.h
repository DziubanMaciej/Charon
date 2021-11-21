#pragma once

#include "charon/watcher/directory_watcher.h"

#include <atomic>

struct inotify_event;

class DirectoryWatcherLinux : public DirectoryWatcher {
public:
    DirectoryWatcherLinux(const std::filesystem::path &directoryPath, FileEventQueue &outputQueue, FileEventQueue &deferredOutputQueue);
    ~DirectoryWatcherLinux() override;

    bool start() override;
    bool stop() override;
    bool isWorking() const override;

private:
    static void watcherThreadProcedure(DirectoryWatcherLinux &watcher);
    static void dummySignalHandler(int) {}
    bool createFileEvent(const inotify_event &inotifyEvent, FileEvent &outEvent) const;

    OsHandle inotifyEventQueue = defaultOsHandle;
    OsHandle inotifyWatchDescriptor = defaultOsHandle;
    std::unique_ptr<std::thread> watcherThread = nullptr;
    std::atomic_bool watcherThreadWorking = false;
};
