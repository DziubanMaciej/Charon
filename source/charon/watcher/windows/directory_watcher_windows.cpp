#include "charon/util/error.h"
#include "charon/watcher/windows/directory_watcher_windows.h"

std::unique_ptr<DirectoryWatcher> DirectoryWatcher::create(const std::filesystem::path &directoryPath, FileEventQueue &outputQueue) {
    return std::unique_ptr<DirectoryWatcher>(new DirectoryWatcherWindows(directoryPath, outputQueue));
}

DirectoryWatcherWindows::~DirectoryWatcherWindows() {
    stop();
}

bool DirectoryWatcherWindows::isWorking() const {
    return directoryHandle != INVALID_HANDLE_VALUE && watcherThread != nullptr;
}

HANDLE DirectoryWatcherWindows::openHandle(const std::filesystem::path &directoryPath) {
    return CreateFileW(
        directoryPath.c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_DELETE | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        nullptr);
}

bool DirectoryWatcherWindows::start() {
    if (isWorking()) {
        return false;
    }

    directoryHandle = openHandle(directoryPath);
    if (directoryHandle == INVALID_HANDLE_VALUE) {
        return false;
    }

    std::atomic_bool startedNotification = false;
    this->watcherThread = std::make_unique<std::thread>(watcherThreadProcedure, std::reference_wrapper{*this}, std::reference_wrapper{startedNotification});
    while (!startedNotification.load())
        ;

    return true;
}

bool DirectoryWatcherWindows::stop() {
    if (!isWorking()) {
        return false;
    }

    CancelIoEx(directoryHandle, nullptr);
    CloseHandle(directoryHandle);
    directoryHandle = INVALID_HANDLE_VALUE;

    watcherThread->join();
    watcherThread = nullptr;
    return true;
}

void DirectoryWatcherWindows::watcherThreadProcedure(DirectoryWatcherWindows &watcher, std::atomic_bool &startedNotification) {
    constexpr size_t bufferSize = 4096;
    auto buffer = std::make_unique<std::byte[]>(bufferSize);

    startedNotification.store(true);

    while (true) {
        // Wait for directory notifications synchronously
        DWORD outputBufferSize{};
        BOOL value = ReadDirectoryChangesW(
            watcher.directoryHandle,
            buffer.get(),
            bufferSize,
            false,
            FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME,
            &outputBufferSize,
            nullptr,
            nullptr);

        // Handle interruption
        if (value == FALSE || outputBufferSize == 0) {
            auto a = GetLastError();
            break;
        }

        // Process returned changes
        auto currentEntry = reinterpret_cast<const FILE_NOTIFY_INFORMATION *>(buffer.get());
        while (true) {
            watcher.outputQueue.push(watcher.createFileEvent(*currentEntry));

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
