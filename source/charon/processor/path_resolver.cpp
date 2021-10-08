#include "charon/processor/path_resolver.h"
#include "charon/util/error.h"
#include "charon/util/filesystem.h"

#include <sstream>

PathResolver::PathResolver(Filesystem &filesystem)
    : filesystem(filesystem) {}

bool PathResolver::validateNameForResolve(const std::string &name) {
    return false;
}

std::filesystem::path PathResolver::resolvePath(const std::filesystem::path &dir,
                                                const std::filesystem::path &oldName,
                                                const std::string &newName,
                                                const std::filesystem::path &lastResolvedPath) const {
    std::string result = newName;

    // Get extension
    const std::filesystem::path extension = oldName.extension();

    // Perform variable substitutions
    replace(result, "${name}", oldName.stem().string());
    replace(result, "${previousName}", lastResolvedPath.stem().string());
    replace(result, "${extension}", removeLeadingDot(extension));

    // If no counter is present, then we're done
    const size_t digits = std::count(newName.begin(), newName.end(), '#');
    if (digits == 0) {
        return finalizePath(dir, result, extension);
    }

    // If we have a counter, try to find lowest available one
    std::string resultWithCounter = result;
    const auto counterStart = resultWithCounter.begin() + resultWithCounter.find('#');
    const size_t maxIndex = getMaxIndex(digits);
    for (size_t index = 0u; index <= maxIndex; index++) {
        const std::string counterString = counterToString(index, digits);
        std::copy_n(counterString.data(), digits, counterStart);

        const std::filesystem::path finalizedResultWithCounter = finalizePath(dir, resultWithCounter, extension);
        const bool available = !filesystem.exists(finalizedResultWithCounter);
        if (available) {
            return finalizedResultWithCounter;
        }
    }

    // We haven't find an available counter
    UNREACHABLE_CODE; // TODO handle this
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
    stream << std::setfill('0') << std::setw(digits) << index;
    return stream.str();
}

std::string PathResolver::removeLeadingDot(const std::filesystem::path &path) {
    if (path.empty()) {
        return {};
    } else {
        return path.string().substr(1);
    }
}

std::filesystem::path PathResolver::finalizePath(const std::filesystem::path &destinationDir,
                                                 const std::filesystem::path &newName,
                                                 const std::filesystem::path &extension) {
    return (destinationDir / newName).replace_extension(extension);
}
