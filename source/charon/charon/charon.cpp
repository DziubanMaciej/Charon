#include "charon/charon/charon.h"
#include "charon/processor/processor_config.h"
#include "charon/util/logger.h"
#include "charon/watcher/directory_watcher_factory.h"

#include <unordered_set>

namespace std {
template <>
struct hash<fs::path> {
    size_t operator()(const fs::path &path) const noexcept {
        // TODO this is not safe, path should be canonized before calculating the hash
        return fs::hash_value(path);
    }
};
} // namespace std

Charon::Charon(const ProcessorConfig &config, Filesystem &filesystem, Logger &logger, DirectoryWatcherFactory &watcherFactory)
    : processor(config, this->eventQueue, filesystem, logger),
      logger(logger) {

    std::unordered_set<fs::path> directoriesToWatch = {};
    for (const ProcessorActionMatcher &matcher : config.matchers) {
        directoriesToWatch.insert(matcher.watchedFolder);
    }

    for (const fs::path &directoryToWatch : directoriesToWatch) {
        this->directoryWatchers.push_back(watcherFactory.create(directoryToWatch, this->eventQueue));
    }
}

bool Charon::start() {
    if (isStarted.exchange(true)) {
        return false;
    }

    // Run watchers
    for (auto watcherIndex = 0u; watcherIndex < directoryWatchers.size(); watcherIndex++) {
        DirectoryWatcher &currentWatcher = *directoryWatchers[watcherIndex];
        if (!currentWatcher.start()) {
            log(logger, LogLevel::Error) << "Watcher for directory " << currentWatcher.getWatchedDirectory() << " failed to start";

            // If any watcher failed to start, stop all those already running
            for (auto startedWatcherIndex = 0u; startedWatcherIndex < directoryWatchers.size(); startedWatcherIndex++) {
                directoryWatchers[startedWatcherIndex]->stop();
            }
            return false;
        }
    }

    // Run processor
    processorThread = std::make_unique<std::thread>([this]() {
        processor.run();
    });

    return true;
}

bool Charon::stop() {
    if (!isStarted.exchange(false)) {
        return false;
    }

    // Stop processor
    FileEvent interruptEvent{};
    interruptEvent.type = FileEvent::Type::Interrupt;
    eventQueue.push(std::move(interruptEvent));
    processorThread->join();
    processorThread = nullptr;

    // Stop watchers
    for (auto &watcher : this->directoryWatchers) {
        watcher->stop();
    }

    return true;
}

void Charon::readUserConsoleInput() {
    std::string line{};
    while (true) {
        std::getline(std::cin, line);

        if (line == "q") {
            stop();
            break;
        }
    }
}
