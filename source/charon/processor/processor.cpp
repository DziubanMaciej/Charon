
#include "charon/processor/processor.h"
#include "charon/util/error.h"
#include "charon/util/filesystem.h"
#include "charon/util/logger.h"
#include "charon/util/string_helper.h"

Processor::Processor(const ProcessorConfig &config, FileEventQueue &eventQueue, Filesystem &filesystem, Logger &logger)
    : pathResolver(filesystem),
      config(config),
      eventQueue(eventQueue),
      filesystem(filesystem),
      logger(logger) {}

void Processor::run() {
    while (true) {
        FileEvent event{};
        if (!eventQueue.blockingPop(event)) {
            break;
        }
        if (event.type == FileEvent::Type::Interrupt) {
            break;
        }
        processEvent(event);
    }
}

void Processor::processEvent(const FileEvent &event) {
    if (auto it = std::find(eventsToIgnore.begin(), eventsToIgnore.end(), event); it != eventsToIgnore.end()) {
        eventsToIgnore.erase(it);
        return;
    }

    if (filesystem.isDirectory(event.path)) {
        return;
    }

    const ProcessorActionMatcher *matcher = findActionMatcher(event);
    if (matcher == nullptr) {
        log(logger, LogLevel::Info) << "Processor could not match file " << event.path << " to any action matcher";
        return;
    }

    ActionMatcherState actionMatcherState{};
    for (const ProcessorAction &action : matcher->actions) {
        if (shouldActionBeExecutedForGivenEventType(event.type, action.type)) {
            executeProcessorAction(event, action, actionMatcherState);
        }
    }
}

const ProcessorActionMatcher *Processor::findActionMatcher(const FileEvent &event) const {
    for (const ProcessorActionMatcher &matcher : this->config.matchers) {
        // Filter by watched folder
        if (event.watchedRootPath != matcher.watchedFolder) {
            continue;
        }

        // Filter by file extension
        if (!matcher.watchedExtensions.empty()) {
            const auto extension = StringHelper::removeLeadingDot(event.path.extension());
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
        executeProcessorActionRemove(event, action, actionMatcherState);
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
    const auto dstPath = pathResolver.resolvePath(data.destinationDir, event.path, data.destinationName, actionMatcherState.lastResolvedPath);
    actionMatcherState.lastResolvedPath = dstPath;
    if (dstPath.empty()) {
        log(logger, LogLevel::Error) << "Processor could not resolve destination filename.";
        return;
    }

    const char *verbForLog = isMove ? "moving" : "copying";
    log(logger, LogLevel::Info) << "Processor " << verbForLog << " file " << event.path << " to " << dstPath;

    OptionalError error{};
    if (isMove) {
        error = filesystem.move(event.path, dstPath);
        eventsToIgnore.push_back(FileEvent{event.watchedRootPath, FileEvent::Type::Remove, event.path});
    } else {
        error = filesystem.copy(event.path, dstPath);
    }

    if (error.has_value()) {
        std::error_code code = error.value();
        log(logger, LogLevel::Error) << "Filesystem operation returned code " << code.value() << ": " << code.message();
    }
}

void Processor::executeProcessorActionRemove(const FileEvent &event, const ProcessorAction &action, ActionMatcherState &actionMatcherState) {
    actionMatcherState.lastResolvedPath = std::filesystem::path{};
    log(logger, LogLevel::Info) << "Processor removing file " << event.path;

    const OptionalError error = filesystem.remove(event.path);
    eventsToIgnore.push_back(FileEvent{event.watchedRootPath, FileEvent::Type::Remove, event.path});

    if (error.has_value()) {
        std::error_code code = error.value();
        log(logger, LogLevel::Error) << "Filesystem operation returned code " << code.value() << ": " << code.message();
    }
}

void Processor::executeProcessorActionPrint(const FileEvent &event) const {
    switch (event.type) {
    case FileEvent::Type::Add:
        log(logger, LogLevel::Info) << "File " << event.path << " has been created";
        break;
    case FileEvent::Type::Remove:
        log(logger, LogLevel::Info) << "File " << event.path << " has been removed";
        break;
    case FileEvent::Type::Modify:
        log(logger, LogLevel::Info) << "File " << event.path << " has been modified";
        break;
    case FileEvent::Type::RenameOld:
        log(logger, LogLevel::Info) << "A file has been moved from path " << event.path;
        break;
    case FileEvent::Type::RenameNew:
        log(logger, LogLevel::Info) << "A file has been moved to " << event.path;
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
