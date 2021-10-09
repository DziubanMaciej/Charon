#pragma once

#include "charon/processor/path_resolver.h"
#include "charon/watcher/file_event.h"

struct Filesystem;
struct Logger;
class ProcessorConfig;
class ProcessorActionMatcher;
class ProcessorAction;

class Processor {
public:
    Processor(const ProcessorConfig &config, FileEventQueue &eventQueue, Filesystem &filesystem, Logger &logger);

    void run();

private:
    // This class holds the state for a given action matcher while it's processed. It is destroyed after processing the last action.
    struct ActionMatcherState {
        std::filesystem::path lastResolvedPath;
    };

    void processEvent(const FileEvent &event) const;
    const ProcessorActionMatcher *findActionMatcher(const FileEvent &event) const;
    void executeProcessorAction(const FileEvent &event, const ProcessorAction &action, ActionMatcherState &actionMatcherState) const;
    void logFileEvent(const FileEvent &event) const;

    PathResolver pathResolver;
    const ProcessorConfig &config;
    FileEventQueue &eventQueue;
    Filesystem &filesystem;
    Logger &logger;
};
