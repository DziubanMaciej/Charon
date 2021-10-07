#include "folder_watcher/processor/path_resolver.h"
#include "folder_watcher/util/error.h"

#include <sstream>

bool PathResolver::validateNameForResolve(const std::string &name) {
    return false;
}

std::filesystem::path PathResolver::resolvePath(const std::filesystem::path &dir,
                                                const std::filesystem::path &oldName,
                                                const std::string &newName,
                                                std::filesystem::path &previousName) {
    std::string result = newName;

    // Perform variable substitutions
    replace(result, "${name}", oldName.filename().string());
    replace(result, "${previousName}", previousName.string());
    replace(result, "${extension}", oldName.extension().string());

    // If no counter is present, then we're done
    const size_t digits = std::count(newName.begin(), newName.end(), '#');
    if (digits == 0) {
        return result;
    }

    // If we have a counter, try to find lowest available one
    std::string resultWithCounter = result;
    const auto counterStart = resultWithCounter.begin() + resultWithCounter.find('#');
    const size_t maxIndex = getMaxIndex(digits);
    for (size_t index = 0u; index <= maxIndex; index++) {
        const std::string counterString = counterToString(index, digits);
        std::copy_n(counterString.data(), digits, counterStart);

        const bool available = !std::filesystem::exists(dir / resultWithCounter);
        if (available) {
            return resultWithCounter;
        }
    }

    UNREACHABLE_CODE; // TODO this is an error
}

void PathResolver::replace(std::string &subject, const std::string &search, const std::string &replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
}

size_t PathResolver::getMaxIndex(size_t digits) {
    size_t result = 1;
    for (; digits > 0; digits--) {
        result *= 10;
    }
    return result - 1;
}

std::string PathResolver::counterToString(size_t index, size_t digits) {
    std::ostringstream stream{};
    stream << std::setw(digits) << index;
    return stream.str();
}
