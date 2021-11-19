#include "charon/processor/path_resolver.h"
#include "charon/processor/processor_config.h"
#include "charon/processor/processor_config_validator.h"
#include "charon/util/logger.h"
#include "charon/util/string_helper.h"

bool ProcessorConfigValidator::validateConfig(const ProcessorConfig &config) {
    for (const ProcessorActionMatcher &matcher : config.matchers) {
        if (!validateActionMatcher(matcher)) {
            return false;
        }
    }
    return true;
}

bool ProcessorConfigValidator::validateActionMatcher(const ProcessorActionMatcher &actionMatcher) {
    for (const auto &extension : actionMatcher.watchedExtensions) {
        if (!validateExtension(extension)) {
            return false;
        }
    }

    bool foundRemoveAction = false;
    for (const ProcessorAction &action : actionMatcher.actions) {
        if (!validateAction(action, foundRemoveAction)) {
            return false;
        }
    }

    return true;
}

bool ProcessorConfigValidator::validateExtension(const fs::path &extension) {
    const auto extensionStr = extension.generic_string<PathCharType>();

    if (!validatePath(extensionStr, "One of watched extensions", true)) {
        return false;
    }

    if (extensionStr.find('.') != extensionStr.npos) {
        log(LogLevel::Error) << "One of watched extensions contains a dot.";
        return false;
    }

    return true;
}

bool ProcessorConfigValidator::validateAction(const ProcessorAction &action, bool &foundRemoveAction) {
    if (foundRemoveAction && action.isFilesystemAction()) {
        log(LogLevel::Error) << "Placing actions after a move/remove action is illegal.";
        return false;
    }

    if (action.isRemovingFile()) {
        foundRemoveAction = true;
    }

    if (action.type == ProcessorAction::Type::Move || action.type == ProcessorAction::Type::Copy) {
        const auto data = std::get<ProcessorAction::MoveOrCopy>(action.data);
        if (!validatePath(data.destinationDir, "Destination directory", false)) {
            return false;
        }
        if (!validatePath(data.destinationName, "Destination name", false)) {
            return false;
        }
        if (!PathResolver::validateNameForResolve(data.destinationName)) {
            return false;
        }
    }

    return true;
}

bool ProcessorConfigValidator::validatePath(const fs::path &path, const std::string &label, bool allowEmpty) {
    if (!allowEmpty && path.empty()) {
        log(LogLevel::Error) << label << " is empty.";
        return false;
    }

    PathStringType pathStr = path.generic_string<PathCharType>();
    for (PathCharType c : pathStr) {
        if (static_cast<unsigned char>(c) >= 128) {
            log(LogLevel::Error) << label << " contains illegal characters.";
            return false;
        }
    }
    return true;
}
