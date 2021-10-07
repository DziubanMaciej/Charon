#pragma once

#include "charon/threading/blocking_queue.h"
#include "charon/watcher/file_action.h"

class ProcessorConfig;
class ProcessorActionMatcher;
class ProcessorAction;

class Processor {
public:
    Processor(ProcessorConfig &config, BlockingQueue<FileAction> &eventQueue);

    void run();

private:
    void processEvent(const FileAction &event) const;
    ProcessorActionMatcher *findActionMatcher(const FileAction &action) const;
    void executeProcessorAction(const FileAction &event, const ProcessorAction &action) const;

    ProcessorConfig &config;
    BlockingQueue<FileAction> &eventQueue;
};
