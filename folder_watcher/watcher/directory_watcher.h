#pragma once

#include "folder_watcher/threading/blocking_queue.h"
#include "folder_watcher/watcher/file_action.h"

#include <thread>

class DirectoryWatcher {
public:
    DirectoryWatcher(const std::filesystem::path &directoryPath, BlockingQueue<FileAction> &outputQueue);
    virtual ~DirectoryWatcher() {}

    static std::unique_ptr<DirectoryWatcher> create(const std::filesystem::path &directoryPath, BlockingQueue<FileAction> &outputQueue);

    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual bool isWorking() const = 0;

protected:
    const std::filesystem::path directoryPath;
    BlockingQueue<FileAction> &outputQueue;
};
