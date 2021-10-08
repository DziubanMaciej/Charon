#include "charon/processor/path_resolver.h"
#include "charon/processor/processor.h"
#include "charon/processor/processor_config.h"
#include "charon/util/error.h"

Processor::Processor(ProcessorConfig &config, BlockingQueue<FileEvent> &eventQueue)
    : config(config),
      eventQueue(eventQueue) {}

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

void Processor::processEvent(const FileEvent &event) const {
    if (event.type != FileEvent::Type::Add) {
        return;
    }

    ProcessorActionMatcher *matcher = findActionMatcher(event);
    if (matcher == nullptr) {
        // TODO log info about unmatched file
        return;
    }

    ActionMatcherState actionMatcherState{};
    for (const ProcessorAction &action : matcher->actions) {
        executeProcessorAction(event, action, actionMatcherState);
    }
}

ProcessorActionMatcher *Processor::findActionMatcher(const FileEvent &event) const {
    for (ProcessorActionMatcher &matcher : this->config.matchers) {
        // Filter by watched folder
        if (event.watchedRootPath != matcher.watchedFolder) {
            continue;
        }

        // Filter by file extension
        if (!matcher.watchedExtensions.empty()) {
            const auto extension = event.path.extension();
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

void Processor::executeProcessorAction(const FileEvent &event, const ProcessorAction &action, ActionMatcherState &actionMatcherState) const {
    switch (action.type) {
    case ProcessorAction::Type::Copy: {
        const auto data = std::get<ProcessorAction::MoveOrCopy>(action.data);
        const auto dstPath = PathResolver::resolvePath(data.destinationDir, event.path, data.destinationName, actionMatcherState.lastResolvedPath);
        std::filesystem::copy(event.path, dstPath);
        actionMatcherState.lastResolvedPath = dstPath;
        break;
    }
    case ProcessorAction::Type::Move: {
        const auto data = std::get<ProcessorAction::MoveOrCopy>(action.data);
        const auto dstPath = PathResolver::resolvePath(data.destinationDir, event.path, data.destinationName, actionMatcherState.lastResolvedPath);
        std::filesystem::rename(event.path, dstPath);
        actionMatcherState.lastResolvedPath = dstPath;
        break;
    }
    case ProcessorAction::Type::Remove:
        std::filesystem::remove(event.path);
        actionMatcherState.lastResolvedPath = std::filesystem::path{};
        break;
    default:
        UNREACHABLE_CODE
    }
}
