#pragma once

#include "charon/util/blocking_queue.h"

#include <filesystem>

struct FileEvent {
    enum class Type {
        Add,
        Remove,
        Modify,
        RenameOld,
        RenameNew,
        Interrupt,
    };
    std::filesystem::path watchedRootPath = {};
    Type type = Type::Add;
    std::filesystem::path path = {};
};

using FileEventQueue = BlockingQueue<FileEvent>;



namespace std {
inline std::string getFileEventTypeAsPassiveVerb(FileEvent::Type type) {
    const static std::string strings[] = {
        "added",
        "removed",
        "modified",
        "RenameOld",
        "RenameNew",
        "Interrupt",
    };
    return strings[static_cast<size_t>(type)];
}
} // namespace std
