
#include "charon/processor/processor.h"
#include "charon/util/error.h"
#include "charon/util/filesystem.h"
#include "charon/util/logger.h"
#include "charon/util/string_helper.h"

#include <algorithm>

Processor::Processor(const ProcessorConfig &config, FileEventQueue &eventQueue, Filesystem &filesystem)
    : pathResolver(filesystem),
      config(config),
      eventQueue(eventQueue),
      filesystem(filesystem) {}

void Processor::run() {
    while (true) {
        FileEvent event{};
        if (!eventQueue.blockingPop(event)) {
            break;
        }
        if (event.isInterrupt()) {
            break;
        }
        processEvent(event);
    }
}

void Processor::processEvent(FileEvent &event) {
    if (event.isLocked()) {
        filesystem.unlockFile(event.lockedFileHandle);
    }

    if (auto it = std::find(eventsToIgnore.begin(), eventsToIgnore.end(), event); it != eventsToIgnore.end()) {
        eventsToIgnore.erase(it);
        return;
    }

    if (filesystem.isDirectory(event.path)) {
        return;
    }

    if (auto matchers = config.matchers(); matchers != nullptr) {
        processEventMatchers(*matchers, event);
    } else if (auto actions = config.actions(); actions != nullptr) {
        processEventActions(*actions, event);
    } else {
        FATAL_ERROR("Invalid processor config type");
    }
}

void Processor::processEventMatchers(const ProcessorConfig::Matchers &configData, FileEvent &event) {
    const ProcessorActionMatcher *matcher = findActionMatcher(configData, event);
    if (matcher == nullptr) {
        log(LogLevel::Info) << "Processor could not match file " << event.path << " to any action matcher";
        return;
    }

    ActionMatcherState actionMatcherState{};
    for (const ProcessorAction &action : matcher->actions) {
        if (shouldActionBeExecutedForGivenEventType(event.type, action.type)) {
            executeProcessorAction(event, action, actionMatcherState);
        }
    }
}

void Processor::processEventActions(const ProcessorConfig::Actions &configData, FileEvent &event) {
    ActionMatcherState actionMatcherState{};
    for (const ProcessorAction &action : configData.actions) {
        executeProcessorAction(event, action, actionMatcherState);
    }
}

const ProcessorActionMatcher *Processor::findActionMatcher(const ProcessorConfig::Matchers &configData, const FileEvent &event) const {
    for (const ProcessorActionMatcher &matcher : configData.matchers) {
        // Filter by watched folder
        if (event.watchedRootPath != matcher.watchedFolder) {
            continue;
        }

        // Filter by file extension
        if (!matcher.watchedExtensions.empty()) {
            const auto extension = StringHelper<PathCharType>::removeLeadingDot(event.path.extension());
            const auto it = std::find(matcher.watchedExtensions.begin(), matcher.watchedExtensions.end(), extension);
            if (it == matcher.watchedExtensions.end()) {
                continue;
            }
        }

        // If we haven't filtered this matcher at this point, we can select it.
        return &matcher;
    }

    // We failed to find matching object
    return nullptr;
}

void Processor::executeProcessorAction(const FileEvent &event, const ProcessorAction &action, ActionMatcherState &actionMatcherState) {
    switch (action.type) {
    case ProcessorAction::Type::Copy:
        executeProcessorActionMoveOrCopy(event, action, actionMatcherState, false);
        break;
    case ProcessorAction::Type::Move:
        executeProcessorActionMoveOrCopy(event, action, actionMatcherState, true);
        break;
    case ProcessorAction::Type::Remove:
        executeProcessorActionRemove(event, actionMatcherState);
        break;
    case ProcessorAction::Type::Print:
        executeProcessorActionPrint(event);
        break;
    default:
        UNREACHABLE_CODE
    }
}

void Processor::executeProcessorActionMoveOrCopy(const FileEvent &event, const ProcessorAction &action,
                                                 ActionMatcherState &actionMatcherState, bool isMove) {
    const auto data = std::get<ProcessorAction::MoveOrCopy>(action.data);
    const auto dstPath = pathResolver.resolvePath(data.destinationDir, event.path, data.destinationName,
                                                  actionMatcherState.lastResolvedPath, data.counterStart);
    actionMatcherState.lastResolvedPath = dstPath;
    if (dstPath.empty()) {
        log(LogLevel::Error) << "Processor could not resolve destination filename.";
        return;
    }

    const char *verbForLog = isMove ? "moving" : "copying";
    log(LogLevel::Info) << "Processor " << verbForLog << " file " << event.path << " to " << dstPath;

    OptionalError error{};
    if (isMove) {
        error = filesystem.move(event.path, dstPath);
        executeCrossDeviceMoveFallback(event.path, dstPath, error);
        if (!error.has_value()) {
            eventsToIgnore.push_back(FileEvent{event.watchedRootPath, FileEvent::Type::Remove, event.path});
        }
    } else {
        error = filesystem.copy(event.path, dstPath);
    }

    if (error.has_value()) {
        std::error_code code = error.value();
        log(LogLevel::Error) << "Filesystem operation returned code " << code.value() << ": " << code.message();
    } else {
        log(LogLevel::VerboseInfo) << "Operation succeeded";
    }
}

void Processor::executeCrossDeviceMoveFallback(const fs::path &src, const fs::path &dst, OptionalError &error) {
    if (error.has_value() && error.value() == std::errc::cross_device_link) {
        error = filesystem.copy(src, dst);
        if (error.has_value()) {
            return;
        }

        error = filesystem.remove(src);
        if (error.has_value()) {
            log(LogLevel::Error) << "Move operation had to be emulated with copy+remove. Remove operation failed";
            return;
        }

        log(LogLevel::Info) << "Move operation had to be emulated with copy+remove.";
    }
}

void Processor::executeProcessorActionRemove(const FileEvent &event, ActionMatcherState &actionMatcherState) {
    actionMatcherState.lastResolvedPath = std::filesystem::path{};
    log(LogLevel::Info) << "Processor removing file " << event.path;

    const OptionalError error = filesystem.remove(event.path);
    eventsToIgnore.push_back(FileEvent{event.watchedRootPath, FileEvent::Type::Remove, event.path});

    if (error.has_value()) {
        std::error_code code = error.value();
        log(LogLevel::Error) << "Filesystem operation returned code " << code.value() << ": " << code.message();
    } else {
        log(LogLevel::VerboseInfo) << "Operation succeeded";
    }
}

void Processor::executeProcessorActionPrint(const FileEvent &event) const {
    switch (event.type) {
    case FileEvent::Type::Add:
        log(LogLevel::Info) << "File " << event.path << " has been created";
        break;
    case FileEvent::Type::Remove:
        log(LogLevel::Info) << "File " << event.path << " has been removed";
        break;
    case FileEvent::Type::Modify:
        log(LogLevel::Info) << "File " << event.path << " has been modified";
        break;
    case FileEvent::Type::RenameOld:
        log(LogLevel::Info) << "A file has been moved from path " << event.path;
        break;
    case FileEvent::Type::RenameNew:
        log(LogLevel::Info) << "A file has been moved to " << event.path;
        break;
    case FileEvent::Type::Interrupt:
        break;
    }
}

bool Processor::shouldActionBeExecutedForGivenEventType(FileEvent::Type eventType, ProcessorAction::Type actionType) {
    const bool isNewFile = eventType == FileEvent::Type::Add || eventType == FileEvent::Type::RenameNew;
    const bool isPrintAction = actionType == ProcessorAction::Type::Print;
    return isPrintAction || isNewFile;
}
