#include "charon/processor/path_resolver.h"
#include "charon/processor/processor_config.h"
#include "charon/processor/processor_config_validator.h"
#include "charon/util/logger.h"
#include "charon/util/string_helper.h"

bool ProcessorConfigValidator::validateConfig(const ProcessorConfig &config) {
    if (auto matchers = config.matchers(); matchers != nullptr) {
        for (const ProcessorActionMatcher &matcher : matchers->matchers) {
            if (!validateActionMatcher(matcher)) {
                return false;
            }
        }
    } else if (auto actions = config.actions(); actions != nullptr) {
        validateActions(actions->actions);
    } else {
        FATAL_ERROR("Invalid processor config type");
    }

    return true;
}

bool ProcessorConfigValidator::validateActionMatcher(const ProcessorActionMatcher &actionMatcher) {
    for (const auto &extension : actionMatcher.watchedExtensions) {
        if (!validateExtension(extension)) {
            return false;
        }
    }
    return validateActions(actionMatcher.actions);
}

bool ProcessorConfigValidator::validateExtension(const fs::path &extension) {
    const auto extensionStr = extension.generic_string<PathCharType>();

    if (!validatePath(extensionStr, "One of watched extensions", true, true)) {
        return false;
    }

    if (extensionStr.find('.') != extensionStr.npos) {
        log(LogLevel::Error) << "One of watched extensions contains a dot.";
        return false;
    }

    return true;
}

bool ProcessorConfigValidator::validateActions(const std::vector<ProcessorAction> &actions) {
    bool foundRemoveAction = false;
    for (const ProcessorAction &action : actions) {
        if (!validateAction(action, foundRemoveAction)) {
            return false;
        }
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
        if (!validatePath(data.destinationDir, "Destination directory", false, true)) {
            return false;
        }
        if (!validatePath(data.destinationName, "Destination name", false, true)) {
            return false;
        }
        if (!PathResolver::validateNameForResolve(data.destinationName)) {
            return false;
        }
    }

    return true;
}

bool ProcessorConfigValidator::validatePath(const fs::path &path, const std::string &label, bool allowEmpty, bool allowNonExistant) {
    if (!allowEmpty && path.empty()) {
        log(LogLevel::Error) << label << " is empty.";
        return false;
    }

    if (!allowNonExistant && !std::filesystem::exists(path)) {
        log(LogLevel::Error) << label << " does not exist.";
        return false;
    }

    using PathUnsignedCharType = std::make_unsigned_t<PathCharType>;
    PathStringType pathStr = path.generic_string<PathCharType>();
    for (PathCharType c : pathStr) {
        if (static_cast<PathUnsignedCharType>(c) >= 128) {
            log(LogLevel::Error) << label << " contains illegal characters.";
            return false;
        }
    }
    return true;
}
