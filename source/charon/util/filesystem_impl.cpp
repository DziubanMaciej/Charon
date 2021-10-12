#include "filesystem_impl.h"

void FilesystemImpl::copy(const fs::path &src, const fs::path &dst) const {
    fs::create_directories(dst.parent_path());
    fs::copy(src, dst);
}

void FilesystemImpl::move(const fs::path &src, const fs::path &dst) const {
    fs::create_directories(dst.parent_path());
    fs::rename(src, dst);
}

void FilesystemImpl::remove(const fs::path &file) const {
    fs::remove(file);
}

bool FilesystemImpl::exists(const fs::path &file) const {
    return fs::exists(file);
}
