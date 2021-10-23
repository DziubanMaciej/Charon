#include "filesystem_impl.h"

OptionalError FilesystemImpl::copy(const fs::path &src, const fs::path &dst) const {
    fs::create_directories(dst.parent_path());

    std::error_code error{};
    auto options = std::filesystem::copy_options::overwrite_existing;
    fs::copy(src, dst, options, error);
    if (error.value() != 0) {
        return error;
    } else {
        return {};
    }
}

OptionalError FilesystemImpl::move(const fs::path &src, const fs::path &dst) const {
    fs::create_directories(dst.parent_path());

    std::error_code error{};
    fs::rename(src, dst, error);
    if (error.value() != 0) {
        return error;
    } else {
        return {};
    }
}

OptionalError FilesystemImpl::remove(const fs::path &file) const {
    std::error_code error{};
    fs::remove(file, error);
    if (error.value() != 0) {
        return error;
    } else {
        return {};
    }
}

bool FilesystemImpl::isDirectory(const fs::path &path) const {
    return fs::is_directory(path);
}

std::vector<fs::path> FilesystemImpl::listFiles(const fs::path &directory) const {
    if (!fs::exists(directory)) {
        return {};
    }

    std::vector<fs::path> result{};
    for (const auto &entry : fs::directory_iterator(directory)) {
        result.push_back(entry.path());
    }
    return result;
}
