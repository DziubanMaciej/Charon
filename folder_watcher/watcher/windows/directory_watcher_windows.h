#pragma once

#include "folder_watcher/watcher/directory_watcher.h"

#include <Windows.h>

class DirectoryWatcherWindows : public DirectoryWatcher {
public:
    using DirectoryWatcher::DirectoryWatcher;
    DirectoryWatcherWindows::~DirectoryWatcherWindows() override;

    bool start() override;
    bool stop() override;
    bool isWorking() const override;

private:
    static HANDLE openHandle(const std::filesystem::path &directoryPath);
    static void watcherThreadProcedure(DirectoryWatcherWindows &watcher);
    FileAction createFileAction(const FILE_NOTIFY_INFORMATION &notifyInfo) const;

    HANDLE directoryHandle = INVALID_HANDLE_VALUE;
    std::unique_ptr<std::thread> watcherThread = nullptr;
};
