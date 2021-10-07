#pragma once

#include <filesystem>

struct FileEvent {
    enum class Type {
        Add,
        Remove,
        Modify,
        RenameOld,
        RenameNew,
    };
    std::filesystem::path watchedRootPath = {};
    Type type = Type::Add;
    std::filesystem::path path = {};
};
