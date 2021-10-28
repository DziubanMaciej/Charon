#pragma once

#include "charon/util/filesystem.h"

#include <variant>
#include <vector>

struct ProcessorAction {
    enum class Type {
        Invalid,
        Copy,
        Move,
        Remove,
        Print,
    };

    struct MoveOrCopy {
        fs::path destinationDir;
        fs::path destinationName;
    };
    struct Remove {};
    struct Print {};

    Type type = Type::Invalid;
    std::variant<MoveOrCopy, Remove, Print> data{};

    bool isRemovingFile() const { return type == Type::Remove || type == Type::Move; }
    bool isFilesystemAction() const { return type == Type::Remove || type == Type::Move || type == Type::Copy; }
};

struct ProcessorActionMatcher {
    std::filesystem::path watchedFolder;
    std::vector<std::filesystem::path> watchedExtensions;
    std::vector<ProcessorAction> actions;
};

struct ProcessorConfig {
    size_t version;
    std::vector<ProcessorActionMatcher> matchers;
};
