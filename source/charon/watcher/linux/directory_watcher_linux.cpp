#include "charon/util/linux/error.h"
#include "charon/watcher/directory_watcher_factory.h"
#include "charon/watcher/linux/directory_watcher_linux.h"

#include <sys/inotify.h>
#include <sys/select.h>
#include <unistd.h>

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
    return watcherThreadWorking.load();
}

bool DirectoryWatcherLinux::start() {
    if (isWorking()) {
        return false;
    }

    fs::create_directories(directoryPath);

    // Initialize inotify
    inotifyEventQueue = inotify_init();
    FATAL_ERROR_IF_SYSCALL_FAILED(inotifyEventQueue, "Failed inotify_init");
    inotifyWatchDescriptor = inotify_add_watch(inotifyEventQueue, directoryPath.c_str(), IN_CLOSE_WRITE | IN_DELETE | IN_MOVE);
    FATAL_ERROR_IF_SYSCALL_FAILED(inotifyWatchDescriptor, "Failed inotify_add_watch");

    // Initialize pipe used to interrupt watcher thread from the main thread
    FATAL_ERROR_IF_SYSCALL_FAILED(pipe(this->watcherThreadInterruptPipe), "Failed pipe creation");

    // Start background thread
    this->watcherThread = std::make_unique<std::thread>(watcherThreadProcedure, std::reference_wrapper{*this});
    while (!isWorking())
        ;

    return true;
}

bool DirectoryWatcherLinux::stop() {
    if (!isWorking()) {
        return false;
    }

    // Interrupt background thread and wait for completion
    FATAL_ERROR_IF_SYSCALL_FAILED(write(watcherThreadInterruptPipe[1], "\0", 1), "Failed interrupting watcher thread");
    watcherThread->join();

    // Verify the thread marked its end
    FATAL_ERROR_IF(isWorking(), "Watcher thread is still working after kill");

    // Cleanup inotify queue
    FATAL_ERROR_IF_SYSCALL_FAILED(inotify_rm_watch(inotifyEventQueue, inotifyWatchDescriptor), "Failed inotify_rm_watch");
    FATAL_ERROR_IF_SYSCALL_FAILED(close(inotifyEventQueue), "Failed closing inotify queue");

    // Cleanup thread reference
    watcherThread = nullptr;

    return true;
}

void DirectoryWatcherLinux::watcherThreadProcedure(DirectoryWatcherLinux &watcher) {
    constexpr size_t bufferSize = sizeof(inotify_event) * 256;
    auto buffer = std::make_unique<std::byte[]>(bufferSize);

    watcher.watcherThreadWorking.store(true);
    while (true) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(watcher.inotifyEventQueue, &fds);
        FD_SET(watcher.watcherThreadInterruptPipe[0], &fds);
        const int nfds = std::max(watcher.inotifyEventQueue, watcher.watcherThreadInterruptPipe[0]) + 1;
        FATAL_ERROR_IF_SYSCALL_FAILED(select(nfds, &fds, nullptr, nullptr, nullptr), "select() failed");

        if (FD_ISSET(watcher.inotifyEventQueue, &fds)) {
            const ssize_t readResult = read(watcher.inotifyEventQueue, buffer.get(), bufferSize);
            FATAL_ERROR_IF_SYSCALL_FAILED(readResult, "Read from inotify queue failed");

            // We received valid data from inotify and we have to process them
            for (ssize_t positionInBuffer = 0; positionInBuffer < readResult;) {
                const inotify_event &inotifyEvent = reinterpret_cast<inotify_event &>(buffer[positionInBuffer]);

                FileEvent fileEvent{};
                if (watcher.createFileEvent(inotifyEvent, fileEvent)) {
                    watcher.pushEvent(std::move(fileEvent));
                }

                positionInBuffer += sizeof(inotify_event) + inotifyEvent.len;
            }
        }

        if (FD_ISSET(watcher.watcherThreadInterruptPipe[0], &fds)) {
            watcher.watcherThreadWorking.store(false);
            break;
        }
    }
}

bool DirectoryWatcherLinux::createFileEvent(const inotify_event &inotifyEvent, FileEvent &outEvent) const {
    if (inotifyEvent.mask & IN_ISDIR) {
        return false;
    }
    if (inotifyEvent.len == 0) {
        return false;
    }

    FileEvent::Type type{};
    if (inotifyEvent.mask & IN_CLOSE_WRITE) {
        type = FileEvent::Type::Add;
    } else if (inotifyEvent.mask & IN_DELETE) {
        type = FileEvent::Type::Remove;
    } else if (inotifyEvent.mask & IN_MOVED_FROM) {
        type = FileEvent::Type::RenameOld;
    } else if (inotifyEvent.mask & IN_MOVED_TO) {
        type = FileEvent::Type::RenameNew;
    } else if (inotifyEvent.mask & IN_MODIFY) {
        type = FileEvent::Type::Modify;
    } else {
        return false;
    }

    const std::string path{inotifyEvent.name};
    outEvent = FileEvent{this->directoryPath, type, this->directoryPath / path};
    return true;
}