#include "charon/processor/path_resolver.h"
#include "charon/processor/processor.h"
#include "charon/processor/processor_config.h"
#include "charon/util/error.h"

Processor::Processor(ProcessorConfig &config, BlockingQueue<FileAction> &eventQueue)
    : config(config),
      eventQueue(eventQueue) {}

void Processor::run() {
    while (true) {
        FileAction action{};
        if (!eventQueue.blockingPop(action)) {
            break;
        }
        processEvent(action);
    }
}

void Processor::processEvent(const FileAction &event) const {
    if (event.type != FileAction::Type::Add) {
        return;
    }

    ProcessorConfigEntry *matcher = findActionMatcher(event);
    if (matcher == nullptr) {
        // TODO log info about unmatched file
        return;
    }

    for (const ProcessorAction &action : matcher->actions) {
        executeProcessorAction(event, action);
    }
}

ProcessorConfigEntry *Processor::findActionMatcher(const FileAction &action) const {
    for (ProcessorConfigEntry &matcher : this->config.entries) {
        // Filter by watched folder
        if (action.watchedRootPath != matcher.watchedFolder) {
            continue;
        }

        // Filter by file extension
        if (!matcher.watchedExtensions.empty()) {
            const auto extension = action.path.extension();
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

void Processor::executeProcessorAction(const FileAction &event, const ProcessorAction &action) const {
    switch (action.type) {
    case ProcessorAction::Type::Copy: {
        const auto data = std::get<ProcessorAction::MoveOrCopy>(action.data);
        std::filesystem::copy(event.path,
                              PathResolver::resolvePath(data.destinationDir, event.path, data.destinationName, std::filesystem::path{}));
        break;
    }
    case ProcessorAction::Type::Move: {
        const auto data = std::get<ProcessorAction::MoveOrCopy>(action.data);
        std::filesystem::rename(event.path,
                                PathResolver::resolvePath(data.destinationDir, event.path, data.destinationName, std::filesystem::path{}));
        break;
    }
    case ProcessorAction::Type::Remove:
        std::filesystem::remove(event.path);
        break;
    default:
        UNREACHABLE_CODE
    }
}
