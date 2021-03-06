#pragma once

#include "charon/util/filesystem.h"

struct FilesystemImpl : Filesystem {
    OptionalError copy(const fs::path &src, const fs::path &dst) const override;
    OptionalError move(const fs::path &src, const fs::path &dst) const override;
    OptionalError remove(const fs::path &file) const override;
    bool isDirectory(const fs::path &path) const override;
    std::vector<fs::path> listFiles(const fs::path &directory) const override;

    virtual bool isFileLockingSupported() const override;
    virtual std::pair<OsHandle, LockResult> lockFile(const fs::path &path) const override;
    virtual void unlockFile(OsHandle &handle) const override;
};
