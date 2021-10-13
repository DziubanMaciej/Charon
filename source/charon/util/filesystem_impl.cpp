#include "filesystem_impl.h"

void FilesystemImpl::copy(const fs::path &src, const fs::path &dst) const {
    fs::create_directories(dst.parent_path());
    std::error_code ec;
    fs::copy(src, dst, ec);
    if (ec.value() != 0) {
        fs::create_directories(dst.parent_path());
    }
}

void FilesystemImpl::move(const fs::path &src, const fs::path &dst) const {
    fs::create_directories(dst.parent_path());
    std::error_code ec;
    fs::rename(src, dst, ec);
    if (ec.value() != 0) {
        fs::create_directories(dst.parent_path());
    }
}

void FilesystemImpl::remove(const fs::path &file) const {
    fs::remove(file);
}

bool FilesystemImpl::isDirectory(const fs::path &path) const {
    return fs::is_directory(path);
}

std::vector<fs::path> FilesystemImpl::listFiles(const fs::path &directory) const {
    std::vector<fs::path> result{};
    for (const auto &entry : fs::directory_iterator(directory)) {
        result.push_back(entry.path());
    }
    return result;
}
