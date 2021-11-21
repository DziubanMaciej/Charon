#include "charon/util/filesystem_impl.h"

std::pair<OsHandle, FilesystemImpl::LockResult> FilesystemImpl::lockFile(const fs::path &path) const {
    ((void)path);
    return {0, LockResult::Success};
}

void FilesystemImpl::unlockFile(OsHandle &handle) const {
    ((void)handle);
}
