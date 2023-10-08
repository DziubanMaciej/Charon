#include "charon/util/filesystem_impl.h"

bool FilesystemImpl::isFileLockingSupported() const {
    return true;
}

std::pair<OsHandle, FilesystemImpl::LockResult> FilesystemImpl::lockFile(const fs::path &path) const {
    HANDLE handle = CreateFileW(
        path.c_str(),
        GENERIC_READ | GENERIC_WRITE | DELETE, // we get full read/write access
        0u,                                    // we don't share the file with anyone
        nullptr,                               // default security descriptor
        OPEN_EXISTING,                         // do not create new file if it doesn't exist
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    LockResult lockResult = LockResult::Success;
    if (handle == INVALID_HANDLE_VALUE) {
        const DWORD error = GetLastError();
        switch (error) {
        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
            lockResult = LockResult::DoesNotExist;
            break;
        case ERROR_SHARING_VIOLATION:
            lockResult = LockResult::UsedByOtherProcess;
            break;
        case ERROR_ACCESS_DENIED:
            lockResult = LockResult::NoAccess;
            break;
        default:
            lockResult = LockResult::Unknown;
            break;
        }
    }

    return {handle, lockResult};
}

void FilesystemImpl::unlockFile(OsHandle &handle) const {
    if (handle != 0 && handle != INVALID_HANDLE_VALUE) {
        CloseHandle(handle);
    }
    handle = INVALID_HANDLE_VALUE;
}
