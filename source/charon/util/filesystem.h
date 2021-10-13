#pragma once
#include "charon/util/class_traits.h"

#include <filesystem>
#include <optional>

namespace fs = std::filesystem;

using OptionalError = std::optional<std::error_code>;

struct Filesystem : NonCopyableAndMovable {
    virtual ~Filesystem() {}
    virtual OptionalError copy(const fs::path &src, const fs::path &dst) const = 0;
    virtual OptionalError move(const fs::path &src, const fs::path &dst) const = 0;
    virtual OptionalError remove(const fs::path &file) const = 0;
    virtual bool isDirectory(const fs::path &path) const = 0;
    virtual std::vector<fs::path> listFiles(const fs::path &directory) const = 0;
};
