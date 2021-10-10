#pragma once

#include "charon/processor/processor_config.h"
#include "os_tests/test_files_helper.h"

#include <string>

struct ProcessorConfigFixture {
    void SetUp() {
        srcPath = TestFilesHelper::createDirectory("src");
        dstPath = TestFilesHelper::createDirectory("dst");
    }

    ProcessorAction createCopyAction(const std::string &destinationName) {
        return createCopyAction(destinationName, this->dstPath);
    }

    ProcessorAction createCopyAction(const std::string &destinationName, const std::filesystem::path &destinationDir) {
        ProcessorAction::MoveOrCopy data{};
        data.destinationDir = destinationDir;
        data.destinationName = destinationName;
        return ProcessorAction{ProcessorAction::Type::Copy, data};
    }

    ProcessorAction createMoveAction(const std::string &destinationName) {
        ProcessorAction::MoveOrCopy data{};
        data.destinationDir = dstPath;
        data.destinationName = destinationName;
        return ProcessorAction{ProcessorAction::Type::Move, data};
    }

    ProcessorAction createRemoveAction() {
        ProcessorAction::Remove data{};
        return ProcessorAction{ProcessorAction::Type::Remove, data};
    }

    ProcessorConfig createProcessorConfigWithOneMatcher() {
        ProcessorActionMatcher matcher{};
        matcher.watchedFolder = srcPath;
        ProcessorConfig config{};
        config.matchers = {matcher};
        return config;
    }

    std::filesystem::path srcPath{};
    std::filesystem::path dstPath{};
};
