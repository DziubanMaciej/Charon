#include "charon/watcher/directory_watcher.h"

DirectoryWatcher::DirectoryWatcher(const std::filesystem::path &directoryPath, FileEventQueue &outputQueue)
    : directoryPath(directoryPath),
      outputQueue(outputQueue) {}
