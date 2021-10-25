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
        std::filesystem::path destinationDir;
        std::string destinationName;
    };
    struct Remove {};
    struct Print {};

    Type type = Type::Invalid;
    std::variant<MoveOrCopy, Remove, Print> data{};
};

struct ProcessorActionMatcher {
    std::filesystem::path watchedFolder;
    std::vector<std::string> watchedExtensions;
    std::vector<ProcessorAction> actions;
};

struct ProcessorConfig {
    size_t version;
    std::vector<ProcessorActionMatcher> matchers;
};
