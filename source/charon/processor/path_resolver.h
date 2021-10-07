
#pragma once

#include <filesystem>
#include <string>

class PathResolver {
public:
    static bool validateNameForResolve(const std::string &name);
    static std::filesystem::path resolvePath(const std::filesystem::path &dir,
                                             const std::filesystem::path &oldName,
                                             const std::string &newName,
                                             std::filesystem::path &previousName);

private:
    static void replace(std::string &subject, const std::string &search, const std::string &replace);
    static size_t getMaxIndex(size_t digits);

    static std::string counterToString(size_t index, size_t digits);
};
