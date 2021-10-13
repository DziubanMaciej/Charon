#include "charon/processor/path_resolver.h"
#include "charon/util/error.h"
#include "charon/util/filesystem.h"
#include "charon/util/string_helper.h"

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
    StringHelper::replace(result, "${name}", oldName.stem().string());
    StringHelper::replace(result, "${previousName}", lastResolvedPath.stem().string());
    StringHelper::replace(result, "${extension}", StringHelper::removeLeadingDot(extension));

    // If no counter is present, then we're done
    const size_t digits = std::count(newName.begin(), newName.end(), '#');
    if (digits == 0) {
        return finalizePath(dir, result, extension);
    }

    // We have a counter, list all files and sort lexicographically
    std::vector<fs::path> filesInNewDir = filesystem.listFiles(dir);
    std::sort(filesInNewDir.begin(), filesInNewDir.end());

    // Find first possible name (with counter 0)
    std::string resultWithCounter = result;
    const auto counterStart = resultWithCounter.begin() + resultWithCounter.find('#');
    setCounter(counterStart, 0u, digits);
    const auto predicate = [&resultWithCounter](const fs::path &p) { return resultWithCounter == p.stem().string(); };
    auto existingFile = std::find_if(filesInNewDir.begin(), filesInNewDir.end(), predicate);
    if (existingFile == filesInNewDir.end()) {
        return finalizePath(dir, resultWithCounter, extension);
    }
    existingFile++;

    // Check rest of names
    size_t counter = 1u;
    setCounter(counterStart, counter, digits);
    for (; existingFile != filesInNewDir.end(); existingFile++) {
        // If it's not free, go to the next one
        if (predicate(*existingFile)) {
            counter++;
            setCounter(counterStart, counter, digits);
        }
    }

    if (counter < getMaxIndex(digits)) {
        return finalizePath(dir, resultWithCounter, extension);
    }

    return {};
}

size_t PathResolver::getMaxIndex(size_t digits) {
    size_t result = 1;
    for (; digits > 0; digits--) {
        result *= 10;
    }
    return result - 1;
}

void PathResolver::setCounter(std::string::iterator counterAddress, size_t index, size_t digits) {
    std::ostringstream counterStringStream{};
    counterStringStream << std::setfill('0') << std::setw(digits) << index;
    const std::string counterString = counterStringStream.str();

    std::copy_n(counterString.data(), digits, counterAddress);
}

std::filesystem::path PathResolver::finalizePath(const std::filesystem::path &destinationDir,
                                                 const std::filesystem::path &newName,
                                                 const std::filesystem::path &extension) {
    return (destinationDir / newName).replace_extension(extension);
}
