#pragma once

#include "charon/charon/os_handle.h"
#include "charon/util/blocking_queue.h"
#include "charon/util/filesystem.h"

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
    OsHandle lockedFileHandle = defaultOsHandle;

    bool isInterrupt() const { return type == Type::Interrupt; }
    bool needsFileLocking() const { return type == Type::Add || type == Type::Modify || type == Type::RenameNew; }
    bool isLocked() const { return lockedFileHandle != defaultOsHandle; }

    const static FileEvent interruptEvent;
};

const inline FileEvent FileEvent::interruptEvent = {L"", FileEvent::Type::Interrupt};

inline bool operator==(const FileEvent &left, const FileEvent &right) {
    return left.watchedRootPath == right.watchedRootPath &&
           left.type == right.type &&
           left.path == right.path;
}

using FileEventQueue = BlockingQueue<FileEvent>;
