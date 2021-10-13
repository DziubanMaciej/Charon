
#pragma once

#include <filesystem>
#include <string>

class Filesystem;

class PathResolver {
public:
    PathResolver(Filesystem &filesystem);

    static bool validateNameForResolve(const std::string &name);
    std::filesystem::path resolvePath(const std::filesystem::path &dir,
                                      const std::filesystem::path &oldName,
                                      const std::string &newName,
                                      const std::filesystem::path &lastResolvedPath) const;

private:
    static size_t getMaxIndex(size_t digits);

    static void setCounter(std::string::iterator counterAddress, size_t index, size_t digits);

    static std::filesystem::path finalizePath(const std::filesystem::path &destinationDir,
                                              const std::filesystem::path &newName,
                                              const std::filesystem::path &extension);

    Filesystem &filesystem;
};
