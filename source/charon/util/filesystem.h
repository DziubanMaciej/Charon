#pragma once

#include <filesystem>

namespace fs = std::filesystem;

struct Filesystem {
    virtual ~Filesystem() {}
    virtual void copy(const fs::path &src, const fs::path &dst) const = 0;
    virtual void move(const fs::path &src, const fs::path &dst) const = 0;
    virtual void remove(const fs::path &file) const = 0;
    virtual bool exists(const fs::path &file) const = 0;
};