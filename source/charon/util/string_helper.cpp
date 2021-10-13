#include "charon/util/string_helper.h"

std::string StringHelper::removeLeadingDot(const std::filesystem::path &path) {
    if (path.empty()) {
        return {};
    } else {
        return path.string().substr(1);
    }
}

void StringHelper::replace(std::string &subject, const std::string &search, const std::string &replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
}
