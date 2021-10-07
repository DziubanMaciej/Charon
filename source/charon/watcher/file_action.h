#pragma once

#include <filesystem>

struct FileAction {
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