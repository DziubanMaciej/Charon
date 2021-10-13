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

std::filesystem::path PathResolver::resolvePath(const std::filesystem::path &newDir,
                                                const std::filesystem::path &oldName,
                                                const std::string &newName,
                                                const std::filesystem::path &lastResolvedName) const {
    std::string result = newName;
    const std::filesystem::path extension = oldName.extension();

    applyVariableSubstitutions(result, oldName, extension, lastResolvedName);
    applyCounterSubstitution(result, newDir);

    if (result.empty()) {
        return result;
    } else {
        return finalizePath(newDir, result, extension);
    }
}

void PathResolver::applyVariableSubstitutions(std::string &name,
                                              const std::filesystem::path &oldName,
                                              const std::filesystem::path &oldNameExtension,
                                              const std::filesystem::path &lastResolvedName) {
    StringHelper::replace(name, "${name}", oldName.stem().string());
    StringHelper::replace(name, "${previousName}", lastResolvedName.stem().string());
    StringHelper::replace(name, "${extension}", StringHelper::removeLeadingDot(oldNameExtension));
}

void PathResolver::applyCounterSubstitution(std::string &name, const std::filesystem::path &newDir) const {
    // If no counter is present, then we're done
    const size_t digits = std::count(name.begin(), name.end(), '#');
    if (digits == 0) {
        return;
    }

    // We have a counter, list all files and sort lexicographically
    std::vector<fs::path> filesInNewDir = filesystem.listFiles(newDir);
    std::sort(filesInNewDir.begin(), filesInNewDir.end());

    // Define a predicate, which will tell us if a filename with given counter value is already taken
    // The predicate ignores the extension.
    const auto isNameTaken = [&name](const fs::path &p) { return name == p.stem().string(); };

    // Substitute counter equal to 0 to our name
    size_t counter = 0u;
    const auto counterStart = name.begin() + name.find('#');
    setCounter(counterStart, counter, digits);

    // If we don't find a conflicting file, we can just take this name
    auto existingFile = std::find_if(filesInNewDir.begin(), filesInNewDir.end(), isNameTaken);
    if (existingFile == filesInNewDir.end()) {
        return;
    }

    // Check rest of names. The list is sorted, so we don't have to check anything before existingFile.
    existingFile++;
    setCounter(counterStart, ++counter, digits);
    for (; existingFile != filesInNewDir.end(); existingFile++) {
        // If it's taken, go to the next one
        if (isNameTaken(*existingFile)) {
            setCounter(counterStart, ++counter, digits);
        }
    }

    // If the counter is too big, we cannot use it.
    if (counter > getMaxIndex(digits)) {
        name.resize(0);
    }
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
