#pragma once

#include "charon/util/class_traits.h"
#include "charon/util/filesystem.h"
#include "charon/watcher/file_event.h"

#include <vector>

class DeferredFileLocker : NonCopyableAndMovable {
public:
    DeferredFileLocker(FileEventQueue &inputQueue, FileEventQueue &outputQueue, Filesystem &filesystem);

    void run();

private:
    void fetchFromInputQueue();
    bool processEvents();

    std::vector<FileEvent> events{};
    FileEventQueue &inputQueue;
    FileEventQueue &outputQueue;
    Filesystem &filesystem;
    std::chrono::milliseconds fetchTimeout;
};
