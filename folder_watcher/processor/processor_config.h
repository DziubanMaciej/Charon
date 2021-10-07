#pragma once

#include <filesystem>
#include <variant>
#include <vector>

struct ProcessorAction {
    enum class Type {
        Invalid,
        Copy,
        Move,
        Remove,
    };

    struct MoveOrCopy {
        std::filesystem::path destinationDir;
        std::string destinationName;
        bool overwriteExisting;
    };
    struct Remove {};

    Type type = Type::Invalid;
    std::variant<MoveOrCopy, Remove> data{};
};

struct ProcessorConfigEntry {
    std::filesystem::path watchedFolder;
    std::vector<std::string> watchedExtensions;
    std::vector<ProcessorAction> actions;
};

struct ProcessorConfig {
    size_t version;
    std::vector<ProcessorConfigEntry> entries;
};
