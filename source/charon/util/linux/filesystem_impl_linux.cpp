#include "charon/util/filesystem_impl.h"

std::pair<OsHandle, FilesystemImpl::LockResult> FilesystemImpl::lockFile(const fs::path &path) const {
    ((void)path);
    return {-1, LockResult::Unknown};
}

void FilesystemImpl::unlockFile(OsHandle &handle) const {
    ((void)handle);
}
