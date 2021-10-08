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
