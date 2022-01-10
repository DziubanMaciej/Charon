#include "charon/util/error.h"
#include "charon/util/logger.h"
#include "charon/watcher/directory_watcher_factory.h"
#include "charon/watcher/windows/directory_watcher_windows.h"

std::unique_ptr<DirectoryWatcher> DirectoryWatcherFactoryImpl::create(const std::filesystem::path &directoryPath,
                                                                      FileEventQueue &outputQueue,
                                                                      FileEventQueue &deferredOutputQueue) {
    return std::unique_ptr<DirectoryWatcher>(new DirectoryWatcherWindows(directoryPath, outputQueue, deferredOutputQueue));
}

DirectoryWatcherWindows::DirectoryWatcherWindows(const std::filesystem::path &directoryPath,
                                                 FileEventQueue &outputQueue,
                                                 FileEventQueue &deferredOutputQueue)
    : DirectoryWatcher(directoryPath, outputQueue, deferredOutputQueue),
      interruptEvent(true) {}

DirectoryWatcherWindows::~DirectoryWatcherWindows() {
    stop();
}

bool DirectoryWatcherWindows::isWorking() const {
    return watcherThreadWorking.load();
}

HANDLE DirectoryWatcherWindows::openHandle(const std::filesystem::path &directoryPath) {
    return CreateFileW(
        directoryPath.c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        nullptr);
}

bool DirectoryWatcherWindows::start() {
    if (isWorking()) {
        return false;
    }

    fs::create_directories(directoryPath);

    // Create handle to our directory
    directoryHandle = openHandle(directoryPath);
    if (directoryHandle == INVALID_HANDLE_VALUE) {
        return false;
    }

    // Start background thread
    this->watcherThread = std::make_unique<std::thread>(watcherThreadProcedure, std::reference_wrapper{*this});
    while (!isWorking())
        ;

    return true;
}

bool DirectoryWatcherWindows::stop() {
    if (!isWorking()) {
        return false;
    }

    // Interrupt background thread and wait for completion
    SetEvent(interruptEvent);
    watcherThread->join();
    ResetEvent(interruptEvent);

    // Verify the thread marked its end
    FATAL_ERROR_IF(isWorking());

    // Cleanup directory handle
    CancelIoEx(directoryHandle, nullptr);
    CloseHandle(directoryHandle);
    directoryHandle = INVALID_HANDLE_VALUE;

    // Cleanup thread reference
    watcherThread = nullptr;

    return true;
}

void DirectoryWatcherWindows::watcherThreadProcedure(DirectoryWatcherWindows &watcher) {
    constexpr size_t bufferSize = 4096;
    auto buffer = std::make_unique<std::byte[]>(bufferSize);

    Event watcherEvent{false};
    OVERLAPPED overlapped{};
    overlapped.hEvent = watcherEvent;

    while (true) {
        // Wait for directory notifications asynchronously
        DWORD outputBufferSize{};
        BOOL retVal = ReadDirectoryChangesW(
            watcher.directoryHandle,
            buffer.get(),
            bufferSize,
            false,
            FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME,
            nullptr,
            &overlapped,
            nullptr);
        FATAL_ERROR_IF(retVal == FALSE, "ReadDirectoryChangesW failed"); // TODO handle this

        // We can tell main thread, that we've started
        watcher.watcherThreadWorking.store(true);

        // We'll wait for two things at the same time - directory notification and interruption signal.
        // Either one of them will wake us and we will have to act accordingly.
        HANDLE eventsForWait[2] = {watcher.interruptEvent, overlapped.hEvent};
        const DWORD waitResult = WaitForMultipleObjects(2u, eventsForWait, false, INFINITE);
        switch (waitResult) {
        case WAIT_OBJECT_0:
            // Interruption event was signalled - we have to return
            watcher.watcherThreadWorking.store(false);
            return;
        case WAIT_OBJECT_0 + 1:
            // Watcher event was signalled - we have to process the buffer we passed to ReadDirectoryChangesW
            break;
        default:
            FATAL_ERROR();
        }

        // ReadDirectoryChangesW notified us. We still have to check if it succeeded
        retVal = GetOverlappedResult(watcher.directoryHandle, &overlapped, &outputBufferSize, false);
        FATAL_ERROR_IF(retVal == FALSE, "GetOverlappedResult failed"); // TODO handle this

        // Process returned events
        auto currentEntry = reinterpret_cast<const FILE_NOTIFY_INFORMATION *>(buffer.get());
        while (true) {
            FileEvent event = watcher.createFileEvent(*currentEntry);
            watcher.pushEvent(std::move(event));

            if (currentEntry->NextEntryOffset == 0) {
                break;
            }
            currentEntry = reinterpret_cast<const FILE_NOTIFY_INFORMATION *>(reinterpret_cast<uintptr_t>(currentEntry) + currentEntry->NextEntryOffset);
        }
    }
}

FileEvent DirectoryWatcherWindows::createFileEvent(const FILE_NOTIFY_INFORMATION &notifyInfo) const {
    FileEvent::Type type{};
    switch (notifyInfo.Action) {
    case FILE_ACTION_ADDED:
        type = FileEvent::Type::Add;
        break;
    case FILE_ACTION_REMOVED:
        type = FileEvent::Type::Remove;
        break;
    case FILE_ACTION_MODIFIED:
        type = FileEvent::Type::Modify;
        break;
    case FILE_ACTION_RENAMED_OLD_NAME:
        type = FileEvent::Type::RenameOld;
        break;
    case FILE_ACTION_RENAMED_NEW_NAME:
        type = FileEvent::Type::RenameNew;
        break;
    default:
        UNREACHABLE_CODE;
    }

    const std::wstring path{notifyInfo.FileName, notifyInfo.FileNameLength / sizeof(WCHAR)};
    return FileEvent{this->directoryPath, type, this->directoryPath / path};
}
