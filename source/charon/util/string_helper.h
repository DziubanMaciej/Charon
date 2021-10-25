#pragma once

#include "charon/util/class_traits.h"
#include "charon/util/filesystem.h"

#include <string>

template <typename CharT>
struct StringHelper : NonInstantiatable {
    using StringT = std::basic_string<CharT>;

    static StringT removeLeadingDot(const std::filesystem::path &path);

    static void replace(StringT &subject,
                        const StringT &search,
                        const StringT &replace);
};
