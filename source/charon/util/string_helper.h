#pragma once

#include <string>
#include <filesystem>

struct StringHelper {
    static std::string removeLeadingDot(const std::filesystem::path &path);
    static void replace(std::string &subject, const std::string &search, const std::string &replace);
};
