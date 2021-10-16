#pragma once

#include "charon/watcher/directory_watcher.h"

#include <Windows.h>

class DirectoryWatcherWindows : public DirectoryWatcher {
public:
    DirectoryWatcherWindows(const std::filesystem::path &directoryPath, FileEventQueue &outputQueue);
    DirectoryWatcherWindows::~DirectoryWatcherWindows() override;

    bool start() override;
    bool stop() override;
    bool isWorking() const override;

private:
    struct Event {
        Event(bool manualReset) : event(CreateEventW(nullptr, manualReset, false, nullptr)) {}
        ~Event() { CloseHandle(event); }
        operator HANDLE() { return event; }

    private:
        HANDLE event;
    };

    static HANDLE openHandle(const std::filesystem::path &directoryPath);
    static void watcherThreadProcedure(DirectoryWatcherWindows &watcher);
    FileEvent createFileEvent(const FILE_NOTIFY_INFORMATION &notifyInfo) const;

    // Contant data
    Event interruptEvent;

    // Data created for current start() session
    HANDLE directoryHandle = INVALID_HANDLE_VALUE;
    std::unique_ptr<std::thread> watcherThread = nullptr;
    std::atomic_bool watcherThreadWorking = false;
};
