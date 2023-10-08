#pragma once

#include "charon/util/error.h"
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
    enum class Type {
        Matchers,
        Actions,
    };
    struct Matchers {
        std::vector<ProcessorActionMatcher> matchers;
    };
    struct Actions {
        std::vector<ProcessorAction> actions;
    };

    Matchers &createMatchers() { return createVariant<Matchers>(); }
    Actions &createActions() { return createVariant<Actions>(); }
    Matchers *matchers() { return getVariant<Matchers>(); }
    Actions *actions() { return getVariant<Actions>(); }
    const Matchers *matchers() const { return const_cast<ProcessorConfig *>(this)->getVariant<Matchers>(); }
    const Actions *actions() const { return const_cast<ProcessorConfig *>(this)->getVariant<Actions>(); }

private:
    template <typename T>
    T *getVariant() {
        if (std::holds_alternative<T>(data)) {
            return &std::get<T>(data);
        } else {
            return nullptr;
        }
    }
    template <typename T>
    T &createVariant() {
        data = T();
        return std::get<T>(data);
    }

    std::variant<Matchers, Actions> data;
};
