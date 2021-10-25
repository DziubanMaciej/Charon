
#pragma once

#include "charon/util/class_traits.h"
#include "charon/util/filesystem.h"

#include <string>

struct Filesystem;

class PathResolver : NonCopyableAndMovable {
public:
    PathResolver(Filesystem &filesystem);

    static bool validateNameForResolve(const std::filesystem::path &namePattern);

    std::filesystem::path resolvePath(const std::filesystem::path &newDir,
                                      const std::filesystem::path &oldName,
                                      const std::filesystem::path &namePattern,
                                      const std::filesystem::path &lastResolvedName) const;

private:
    static void applyVariableSubstitutions(PathStringType &name,
                                           const std::filesystem::path &oldName,
                                           const std::filesystem::path &oldNameExtension,
                                           const std::filesystem::path &lastResolvedName);
    void applyCounterSubstitution(PathStringType &name,
                                  const std::filesystem::path &newDir) const;

    static size_t getMaxIndex(size_t digits);

    static void setCounter(PathStringType::iterator counterAddress, size_t index, size_t digits);

    static std::filesystem::path finalizePath(const std::filesystem::path &destinationDir,
                                              const PathStringType &name,
                                              const std::filesystem::path &extension);

    Filesystem &filesystem;
};
