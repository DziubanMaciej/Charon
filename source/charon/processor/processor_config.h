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
