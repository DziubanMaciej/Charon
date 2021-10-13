#pragma once

#include "charon/util/filesystem.h"

struct FilesystemImpl : Filesystem {
    void copy(const fs::path &src, const fs::path &dst) const override;
    void move(const fs::path &src, const fs::path &dst) const override;
    void remove(const fs::path &file) const override;
    bool exists(const fs::path &file) const override;
    std::vector<fs::path> listFiles(const fs::path &directory) const override;
};
