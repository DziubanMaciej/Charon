#pragma once

#include "charon/processor/processor_config.h"

#include <gtest/gtest.h>

struct ProcessorConfigFixture {
    ProcessorAction createCopyAction(const std::filesystem::path &destinationDir, const std::string &destinationName, size_t counterStart = 0) {
        ProcessorAction::MoveOrCopy data{};
        data.destinationDir = destinationDir;
        data.destinationName = destinationName;
        data.counterStart = counterStart;
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
        return createProcessorConfigWithMatchers({watchedDir});
    }

    ProcessorConfig createProcessorConfigWithMatchers(std::initializer_list<std::filesystem::path> watchedDirs) {
        ProcessorConfig config{};
        ProcessorConfig::Matchers &matchers = config.createMatchers();
        for (const std::filesystem::path &watcherDir : watchedDirs) {
            matchers.matchers.emplace_back();
            matchers.matchers.back().watchedFolder = watcherDir;
        }
        return config;
    }

    ProcessorConfig createProcessorConfigWithActions(std::initializer_list<ProcessorAction> actionsToAdd) {
        ProcessorConfig config{};
        ProcessorConfig::Actions &actions = config.createActions();
        for (const ProcessorAction &action : actionsToAdd) {
            actions.actions.push_back(action);
        }
        return config;
    }

    const static inline fs::path dummyPath1 = "dummy/path/1/";
    const static inline fs::path dummyPath2 = "dummy/path/2/";
    const static inline fs::path dummyPath3 = "dummy/path/3/";
};
