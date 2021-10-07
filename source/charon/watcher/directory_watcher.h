#pragma once

#include "charon/threading/blocking_queue.h"
#include "charon/watcher/file_action.h"

#include <thread>

class DirectoryWatcher {
public:
    DirectoryWatcher(const std::filesystem::path &directoryPath, BlockingQueue<FileEvent> &outputQueue);
    virtual ~DirectoryWatcher() {}

    static std::unique_ptr<DirectoryWatcher> create(const std::filesystem::path &directoryPath, BlockingQueue<FileEvent> &outputQueue);

    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual bool isWorking() const = 0;

protected:
    const std::filesystem::path directoryPath;
    BlockingQueue<FileEvent> &outputQueue;
};
