#pragma once

#include "charon/util/class_traits.h"
#include "charon/util/filesystem.h"

#include <string>

struct StringHelper : NonInstantiatable {
    static std::string removeLeadingDot(const std::filesystem::path &path);
    static void replace(std::string &subject, const std::string &search, const std::string &replace);
};
