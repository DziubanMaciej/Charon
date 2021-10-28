#pragma once

#include "charon/processor/processor_config.h"

#include <gtest/gtest.h>

struct ProcessorConfigFixture {
    void SetUp() {
        dummyPath1 = std::filesystem::path("dummy/path/1/");
        dummyPath2 = std::filesystem::path("dummy/path/2/");
        dummyPath3 = std::filesystem::path("dummy/path/3/");
    }

    ProcessorAction createCopyAction(const std::filesystem::path &destinationDir, const std::string &destinationName) {
        ProcessorAction::MoveOrCopy data{};
        data.destinationDir = destinationDir;
        data.destinationName = destinationName;
        return ProcessorAction{ProcessorAction::Type::Copy, data};
    }

    ProcessorAction createMoveAction(const std::filesystem::path &destinationDir, const std::string &destinationName) {
        ProcessorAction::MoveOrCopy data{};
        data.destinationDir = destinationDir;
        data.destinationName = destinationName;
        return ProcessorAction{ProcessorAction::Type::Move, data};
    }

    ProcessorAction createRemoveAction() {
        ProcessorAction::Remove data{};
        return ProcessorAction{ProcessorAction::Type::Remove, data};
    }

    ProcessorAction createPrintAction() {
        ProcessorAction::Print data{};
        return ProcessorAction{ProcessorAction::Type::Print, data};
    }

    ProcessorConfig createProcessorConfigWithOneMatcher(const std::filesystem::path &watchedDir) {
        return createProcessorConfig({watchedDir});
    }

    ProcessorConfig createProcessorConfig(std::initializer_list<std::filesystem::path> watchedDirs) {
        ProcessorConfig config{};
        for (const std::filesystem::path &watcherDir : watchedDirs) {
            config.matchers.emplace_back();
            config.matchers.back().watchedFolder = watcherDir;
        }
        return config;
    }

    std::filesystem::path dummyPath1{};
    std::filesystem::path dummyPath2{};
    std::filesystem::path dummyPath3{};
};
