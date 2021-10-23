#include "charon/processor/deferred_file_locker.h"
#include "charon/util/error.h"
#include "charon/util/logger.h"

DeferredFileLocker::DeferredFileLocker(FileEventQueue &inputQueue, FileEventQueue &outputQueue, Filesystem &filesystem)
    : inputQueue(inputQueue),
      outputQueue(outputQueue),
      filesystem(filesystem),
      fetchTimeout(std::chrono::milliseconds(100)) {}

void DeferredFileLocker::run() {
    bool running = true;
    while (running) {
        fetchFromInputQueue();
        running = !processEvents();
    }
}

void DeferredFileLocker::fetchFromInputQueue() {
    FileEvent event{};
    if (inputQueue.blockingPop(event, fetchTimeout)) {
        do {
            events.push_back(std::move(event));
        } while (inputQueue.nonBlockingPop(event));
    }
}

bool DeferredFileLocker::processEvents() {
    bool interrupt = false;
    for (auto it = events.begin(); it != events.end();) {
        bool removeFromFurtherChecks = false;
        bool passToOutputQueue = false;

        if (it->isInterrupt()) {
            // Interrupt event can be handled immediately (just interrupt the loop),
            // but we don't pass it through to the output queue
            interrupt = true;
            removeFromFurtherChecks = true;
            passToOutputQueue = false;
        } else {
            auto [handle, result] = filesystem.lockFile(it->path);
            switch (result) {
            case Filesystem::LockResult::Success:
                // We have access to the file, we can pass it to the processor
                it->lockedFileHandle = handle;
                removeFromFurtherChecks = true;
                passToOutputQueue = true;
                break;
            case Filesystem::LockResult::UsedByOtherProcess:
                // We don't do anything with this file, maybe it won't be used next time we check
                break;
            default:
                // We're giving up on this file, it's been removed or we don't have access
                removeFromFurtherChecks = true;
                passToOutputQueue = false;
            }
        }

        if (removeFromFurtherChecks) {
            FileEvent event = std::move(*it);
            it = events.erase(it);

            if (passToOutputQueue) {
                outputQueue.push(std::move(event));
            }
        } else {
            it++;
        }
    }

    return interrupt;
}
