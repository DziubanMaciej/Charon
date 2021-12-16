#include "charon/util/filesystem_impl.h"

bool FilesystemImpl::isFileLockingSupported() const {
    return false;
}

std::pair<OsHandle, FilesystemImpl::LockResult> FilesystemImpl::lockFile([[maybe_unused]] const fs::path &path) const {
    return {defaultOsHandle, LockResult::NotSupported};
}

void FilesystemImpl::unlockFile([[maybe_unused]] OsHandle &handle) const {}
