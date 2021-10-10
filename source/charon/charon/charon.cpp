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

bool Charon::runWatchers() {
    for (auto &watcher : this->directoryWatchers) {
        const bool watcherStarted = watcher->start();
        if (!watcherStarted) {
            log(logger, LogLevel::Error) << "Watcher for directory " << watcher->getWatchedDirectory() << " failed to start";
            return false;
        }
    }
    return true;
}

void Charon::runProcessor() {
    processor.run();
}

void Charon::readUserConsoleInput() {
    std::string line{};
    while (true) {
        std::getline(std::cin, line);

        if (line == "q") {
            stopProcessor();
            break;
        }
    }
}

void Charon::stopProcessor() {
    FileEvent interruptEvent{};
    interruptEvent.type = FileEvent::Type::Interrupt;
    eventQueue.push(std::move(interruptEvent));
}
