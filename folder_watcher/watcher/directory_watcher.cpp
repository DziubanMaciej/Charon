#include "folder_watcher/watcher/directory_watcher.h"

DirectoryWatcher::DirectoryWatcher(const std::filesystem::path &directoryPath, BlockingQueue<FileAction> &outputQueue)
    : directoryPath(directoryPath),
      outputQueue(outputQueue) {}
