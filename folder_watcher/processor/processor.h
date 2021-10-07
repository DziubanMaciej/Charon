#pragma once

#include "folder_watcher/threading/blocking_queue.h"
#include "folder_watcher/watcher/file_action.h"

class ProcessorConfig;
class ProcessorConfigEntry;
class ProcessorAction;

class Processor {
public:
    Processor(ProcessorConfig &config, BlockingQueue<FileAction> &eventQueue);

    void run();

private:
    void processEvent(const FileAction &event) const;
    ProcessorConfigEntry *findActionMatcher(const FileAction &action) const;
    void executeProcessorAction(const FileAction &event, const ProcessorAction &action) const;

    ProcessorConfig &config;
    BlockingQueue<FileAction> &eventQueue;
};
