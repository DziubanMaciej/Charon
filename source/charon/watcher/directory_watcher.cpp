#include "charon/watcher/directory_watcher.h"

DirectoryWatcher::DirectoryWatcher(const std::filesystem::path &directoryPath, BlockingQueue<FileEvent> &outputQueue)
    : directoryPath(directoryPath),
      outputQueue(outputQueue) {}
