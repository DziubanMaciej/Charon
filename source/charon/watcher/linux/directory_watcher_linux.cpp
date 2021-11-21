#include "charon/util/linux/error.h"
#include "charon/watcher/directory_watcher_factory.h"
#include "charon/watcher/linux/directory_watcher_linux.h"

#include <pthread.h>
#include <signal.h>
#include <sys/inotify.h>
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

    inotifyEventQueue = inotify_init();
    FATAL_ERROR_IF_SYSCALL_FAILED(inotifyEventQueue, "Failed inotify_init");
    inotifyWatchDescriptor = inotify_add_watch(inotifyEventQueue, directoryPath.c_str(), IN_CREATE | IN_DELETE | IN_MOVE | IN_MODIFY);
    FATAL_ERROR_IF_SYSCALL_FAILED(inotifyWatchDescriptor, "Failed inotify_add_watch");

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
    FATAL_ERROR_IF_SYSCALL_FAILED(pthread_kill(watcherThread->native_handle(), SIGUSR1), "Failed pthread_kill");
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

    // We will use SIGUSR1 signal to interrupt read() operation in this thread.
    // We have to still register a signal handler, so the thread is not killed.
    struct sigaction h = {};
    h.sa_handler = dummySignalHandler;
    sigaction(SIGUSR1, &h, 0);

    watcher.watcherThreadWorking.store(true);
    while (true) {
        const ssize_t readResult = read(watcher.inotifyEventQueue, buffer.get(), bufferSize);

        // Handle thread interruption
        if (readResult == -1 && errno == EINTR) {
            watcher.watcherThreadWorking.store(false);
            return;
        }

        // Handle other errors
        FATAL_ERROR_IF_SYSCALL_FAILED(readResult, "Read from inotify queue failed");

        // We received valid data from inotify and we have to process them
        for (ssize_t positionInBuffer = 0; positionInBuffer < readResult;) {
            const inotify_event &inotifyEvent = reinterpret_cast<inotify_event &>(buffer[positionInBuffer]);

            FileEvent fileEvent{};
            if (watcher.createFileEvent(inotifyEvent, fileEvent)) {
                if (fileEvent.needsFileLocking()) {
                    watcher.deferredOutputQueue.push(std::move(fileEvent));
                } else {
                    watcher.outputQueue.push(std::move(fileEvent));
                }
            }

            positionInBuffer += sizeof(inotify_event) + inotifyEvent.len;
        }
    }
}

bool DirectoryWatcherLinux::createFileEvent(const inotify_event &inotifyEvent, FileEvent &outEvent) const {
    if (inotifyEvent.mask & IN_ISDIR) {
        return false;
    }

    FileEvent::Type type{};
    if (inotifyEvent.mask & IN_CREATE) {
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