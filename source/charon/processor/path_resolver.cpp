#include "charon/processor/path_resolver.h"
#include "charon/util/error.h"
#include "charon/util/logger.h"
#include "charon/util/string_helper.h"
#include "charon/util/string_literal.h"

#include <regex>
#include <sstream>

PathResolver::PathResolver(Filesystem &filesystem)
    : filesystem(filesystem) {}

bool PathResolver::validateCounterStartForResolve(const std::filesystem::path &namePattern, size_t counterStart) {
    const PathStringType namePatternStr = namePattern.generic_string<PathCharType>();

    const size_t counterDigits = std::count(namePatternStr.begin(), namePatternStr.end(), '#');
    if (counterDigits > 0) {
        const size_t maxCounterStart = getMaxIndex(counterDigits);
        if (counterStart > maxCounterStart) {
            log(LogLevel::Error) << "counterStart " << counterStart << " is too large for \"" << namePattern << "\". Max is " << maxCounterStart << ".";
            return false;
        }
    } else if (counterStart != 0) {
        log(LogLevel::Error) << "counterStart cannot be specified when counter are not used.";
        return false;
    }

    return true;
}

bool PathResolver::validateNameForResolve(const std::filesystem::path &namePattern) {
    const PathStringType namePatternStr = namePattern.generic_string<PathCharType>();
    std::match_results<PathStringType::const_iterator> result{};

    // Invalid variables
    {
        std::basic_regex<PathCharType> regex{CSTRING(R"(\$\{[^${]*\})")};
        auto namePatternStrBegin = namePatternStr.begin();
        while (namePatternStrBegin < namePatternStr.end()) {
            bool matched = std::regex_search(namePatternStr.begin(), namePatternStr.end(), result, regex);
            if (!matched) {
                break;
            }

            static const PathStringType validVariables[] = {
                CSTRING("${name}"),
                CSTRING("${previousName}"),
                CSTRING("${extension}"),
            };
            const bool isValid = std::find(std::begin(validVariables), std::end(validVariables), result.str()) != std::end(validVariables);
            if (!isValid) {
                log(LogLevel::Error) << "Destination name contains illegal pseudo-variables.";
                return false;
            }

            namePatternStrBegin += result.size();
        }
    }

    // Unclosed variables
    {
        std::basic_regex<PathCharType> regex{CSTRING(R"((\$\{[^${}]*[${])|(\$\{[^}]*$))")};
        if (std::regex_search(namePatternStr.begin(), namePatternStr.end(), result, regex)) {
            log(LogLevel::Error) << "Destination name contains unclosed pseudo-variables.";
            return false;
        }
    }

    // Uncontiguous hashes
    {
        bool foundHash = false;
        bool foundHashInPrevious = false;
        for (auto c : namePatternStr) {
            const bool foundHashInCurrent = c == '#';
            if (foundHashInCurrent) {
                if (foundHash && !foundHashInPrevious) {
                    log(LogLevel::Error) << "Desination name contains uncontiguous hashes.";
                    return false;
                }
                foundHash = true;
            }
            foundHashInPrevious = foundHashInCurrent;
        }
    }

    return true;
}

std::filesystem::path PathResolver::resolvePath(const std::filesystem::path &newDir,
                                                const std::filesystem::path &oldName,
                                                const std::filesystem::path &namePattern,
                                                const std::filesystem::path &lastResolvedName,
                                                size_t counterStart) const {
    PathStringType result = namePattern.generic_string<PathCharType>();
    const std::filesystem::path extension = oldName.extension();

    applyVariableSubstitutions(result, oldName, extension, lastResolvedName);
    applyCounterSubstitution(result, newDir, counterStart);

    if (result.empty()) {
        return result;
    } else {
        return finalizePath(newDir, result, extension);
    }
}

void PathResolver::applyVariableSubstitutions(PathStringType &name,
                                              const std::filesystem::path &oldName,
                                              const std::filesystem::path &oldNameExtension,
                                              const std::filesystem::path &lastResolvedName) {
    StringHelper<PathCharType>::replace(name, CSTRING("${name}"), oldName.stem().generic_string<PathCharType>());
    StringHelper<PathCharType>::replace(name, CSTRING("${previousName}"), lastResolvedName.stem().generic_string<PathCharType>());
    StringHelper<PathCharType>::replace(name, CSTRING("${extension}"), StringHelper<PathCharType>::removeLeadingDot(oldNameExtension));
}

void PathResolver::applyCounterSubstitution(PathStringType &name, const std::filesystem::path &newDir, size_t counterStart) const {
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
    const auto isNameTaken = [&name](const fs::path &p) {
        return name == p.stem().generic_string<PathCharType>();
    };

    // Substitute first counter value in place of hashes
    size_t counter = counterStart;
    const auto counterAddress = name.begin() + name.find('#');
    setCounter(counterAddress, counter, digits);

    // If we don't find a conflicting file, we can just take this name
    auto existingFile = std::find_if(filesInNewDir.begin(), filesInNewDir.end(), isNameTaken);
    if (existingFile == filesInNewDir.end()) {
        return;
    }

    // We have a conflict. Check rest of names. The list is sorted, so we don't have to check anything before existingFile.
    existingFile++;
    setCounter(counterAddress, ++counter, digits);
    for (; existingFile != filesInNewDir.end(); existingFile++) {
        // If it's taken, go to the next one
        if (isNameTaken(*existingFile)) {
            setCounter(counterAddress, ++counter, digits);
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

void PathResolver::setCounter(PathStringType::iterator counterAddress, size_t index, size_t digits) {
    std::ostringstream counterStringStream{};
    counterStringStream << std::setfill('0') << std::setw(digits) << index;
    const std::string counterString = counterStringStream.str();

    std::copy_n(counterString.data(), digits, counterAddress);
}

std::filesystem::path PathResolver::finalizePath(const std::filesystem::path &destinationDir,
                                                 const PathStringType &name,
                                                 const std::filesystem::path &extension) {
    return (destinationDir / name).replace_extension(extension);
}
