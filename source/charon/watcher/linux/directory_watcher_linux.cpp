#include "charon/watcher/directory_watcher_factory.h"
#include "charon/watcher/linux/directory_watcher_linux.h"

std::unique_ptr<DirectoryWatcher> DirectoryWatcherFactoryImpl::create(const std::filesystem::path &directoryPath,
                                                                      FileEventQueue &outputQueue,
                                                                      FileEventQueue &deferredOutputQueue) {
    return std::unique_ptr<DirectoryWatcher>(new DirectoryWatcherLinux(directoryPath, outputQueue, deferredOutputQueue));
}

DirectoryWatcherLinux::DirectoryWatcherLinux(const std::filesystem::path &directoryPath,
                                             FileEventQueue &outputQueue,
                                             FileEventQueue &deferredOutputQueue)
    : DirectoryWatcher(directoryPath, outputQueue, deferredOutputQueue) {}

DirectoryWatcherLinux::~DirectoryWatcherLinux() {
    stop();
}

bool DirectoryWatcherLinux::isWorking() const {
    return false;
}

bool DirectoryWatcherLinux::start() {
    return true;
}

bool DirectoryWatcherLinux::stop() {
    return true;
}
