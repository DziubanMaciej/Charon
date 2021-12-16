#include "charon/watcher/directory_watcher.h"

DirectoryWatcher::DirectoryWatcher(const std::filesystem::path &directoryPath, FileEventQueue &outputQueue, FileEventQueue &deferredOutputQueue)
    : directoryPath(directoryPath),
      outputQueue(outputQueue),
      deferredOutputQueue(deferredOutputQueue) {}

void DirectoryWatcher::pushEvent(FileEvent &&fileEvent) {
    if (fileEvent.needsFileLocking()) {
        deferredOutputQueue.push(std::move(fileEvent));
    } else {
        outputQueue.push(std::move(fileEvent));
    }
}
