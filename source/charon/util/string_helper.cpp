#include "charon/util/string_helper.h"

template <typename CharT>
typename StringHelper<CharT>::StringT StringHelper<CharT>::removeLeadingDot(const std::filesystem::path &path) {
    if (path.empty()) {
        return {};
    } else {
        return path.generic_string<CharT>().substr(1);
    }
}

template <typename CharT>
void StringHelper<CharT>::replace(StringT &subject,
                                  const StringT &search,
                                  const StringT &replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
}

template struct StringHelper<char>;
template struct StringHelper<wchar_t>;
