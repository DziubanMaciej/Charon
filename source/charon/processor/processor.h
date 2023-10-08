#pragma once

#include "charon/processor/path_resolver.h"
#include "charon/processor/processor_config.h"
#include "charon/util/class_traits.h"
#include "charon/watcher/file_event.h"

struct Filesystem;
struct ProcessorConfig;
struct ProcessorActionMatcher;

class Processor : NonCopyableAndMovable {
public:
    Processor(const ProcessorConfig &config, FileEventQueue &eventQueue, Filesystem &filesystem);

    void run();

private:
    // This class holds the state for a given action matcher while it's processed. It is destroyed after processing the last action.
    struct ActionMatcherState {
        std::filesystem::path lastResolvedPath;
    };

    void processEvent(FileEvent &event);
    void processEventMatchers(const ProcessorConfig::Matchers &configData, FileEvent &event);
    void processEventActions(const ProcessorConfig::Actions &configData, FileEvent &event);

    const ProcessorActionMatcher *findActionMatcher(const ProcessorConfig::Matchers &configData, const FileEvent &event) const;
    void executeProcessorAction(const FileEvent &event, const ProcessorAction &action, ActionMatcherState &actionMatcherState);
    void executeProcessorActionMoveOrCopy(const FileEvent &event, const ProcessorAction &action,
                                          ActionMatcherState &actionMatcherState, bool isMove);
    void executeCrossDeviceMoveFallback(const fs::path &src, const fs::path &dst, OptionalError &error);
    void executeProcessorActionRemove(const FileEvent &event, ActionMatcherState &actionMatcherState);
    void executeProcessorActionPrint(const FileEvent &event) const;

    static bool shouldActionBeExecutedForGivenEventType(FileEvent::Type eventType, ProcessorAction::Type actionType);

    PathResolver pathResolver;
    std::vector<FileEvent> eventsToIgnore{};
    const ProcessorConfig &config;
    FileEventQueue &eventQueue;
    Filesystem &filesystem;
};
