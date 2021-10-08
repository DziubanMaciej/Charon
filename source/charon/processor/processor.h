#pragma once

#include "charon/threading/blocking_queue.h"
#include "charon/watcher/file_action.h"

class ProcessorConfig;
class ProcessorActionMatcher;
class ProcessorAction;

class Processor {
public:
    Processor(ProcessorConfig &config, BlockingQueue<FileEvent> &eventQueue);

    void run();

private:
    // This class holds the state for a given action matcher while it's processed. It is destroyed after processing the last action.
    struct ActionMatcherState {
        std::filesystem::path lastResolvedPath;
    };

    void processEvent(const FileEvent &event) const;
    ProcessorActionMatcher *findActionMatcher(const FileEvent &event) const;
    void executeProcessorAction(const FileEvent &event, const ProcessorAction &action, ActionMatcherState &actionMatcherState) const;

    ProcessorConfig &config;
    BlockingQueue<FileEvent> &eventQueue;
};
