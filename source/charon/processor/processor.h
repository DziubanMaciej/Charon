#pragma once

#include "charon/processor/path_resolver.h"
#include "charon/threading/blocking_queue.h"
#include "charon/watcher/file_action.h"

struct Filesystem;
struct Logger;
class ProcessorConfig;
class ProcessorActionMatcher;
class ProcessorAction;

class Processor {
public:
    Processor(ProcessorConfig &config, BlockingQueue<FileEvent> &eventQueue, Filesystem &filesystem, Logger *logger);

    void run();

private:
    // This class holds the state for a given action matcher while it's processed. It is destroyed after processing the last action.
    struct ActionMatcherState {
        std::filesystem::path lastResolvedPath;
    };

    void processEvent(const FileEvent &event) const;
    ProcessorActionMatcher *findActionMatcher(const FileEvent &event) const;
    void executeProcessorAction(const FileEvent &event, const ProcessorAction &action, ActionMatcherState &actionMatcherState) const;

    PathResolver pathResolver;
    ProcessorConfig &config;
    BlockingQueue<FileEvent> &eventQueue;
    Filesystem &filesystem;
    Logger *logger;
};
