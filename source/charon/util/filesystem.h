#pragma once

#include <filesystem>

namespace fs = std::filesystem;

struct Filesystem {
    virtual ~Filesystem() {}
    virtual void copy(const fs::path &src, const fs::path &dst) const = 0;
    virtual void move(const fs::path &src, const fs::path &dst) const = 0;
    virtual void remove(const fs::path &file) const = 0;
    virtual bool isDirectory(const fs::path &path) const = 0;
    virtual std::vector<fs::path> listFiles(const fs::path &directory) const = 0;
};
