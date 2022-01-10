#include "charon/watcher/directory_watcher.h"

DirectoryWatcher::DirectoryWatcher(const std::filesystem::path &directoryPath, FileEventQueue &outputQueue, FileEventQueue &deferredOutputQueue)
    : directoryPath(directoryPath),
      outputQueue(outputQueue),
      deferredOutputQueue(deferredOutputQueue) {}

bool DirectoryWatcher::start() {
    if (isWorking()) {
        return false;
    }

    fs::create_directories(directoryPath);

    if (!startImpl()) {
        return false;
    }

    while (!isWorking())
        ;

    return true;
}

bool DirectoryWatcher::stop() {
    if (!isWorking()) {
        return false;
    }
    return stopImpl();
}

void DirectoryWatcher::pushEvent(FileEvent &&fileEvent) {
    if (fileEvent.needsFileLocking()) {
        deferredOutputQueue.push(std::move(fileEvent));
    } else {
        outputQueue.push(std::move(fileEvent));
    }
}
