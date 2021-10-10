#include "charon/processor/processor.h"
#include "charon/processor/processor_config.h"
#include "charon/util/logger.h"
#include "unit_tests/mocks/mock_filesystem.h"
#include "unit_tests/mocks/mock_logger.h"

#include <gtest/gtest.h>

using ::testing::_;
using ::testing::AtLeast;
using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::Return;

struct ProcessorTest : ::testing::Test {
    void SetUp() override {
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

    void pushFileCreationEvent(const std::filesystem::path &watchedDir, const std::filesystem::path &path) {
        pushFileEvent(watchedDir, FileEvent::Type::Add, path);
    }

    void pushFileEvent(const std::filesystem::path &watchedDir, FileEvent::Type type, const std::filesystem::path &path) {
        eventQueue.push(FileEvent{watchedDir, type, path});
    }

    void pushInterruptEvent() {
        eventQueue.push(FileEvent{std::filesystem::path{}, FileEvent::Type::Interrupt, std::filesystem::path{}});
    }

    std::filesystem::path dummyPath1{};
    std::filesystem::path dummyPath2{};
    std::filesystem::path dummyPath3{};
    FileEventQueue eventQueue{};
    NullLogger nullLogger{};
};

TEST_F(ProcessorTest, whenCopyActionIsTriggeredThenRequestFileCopy) {
    MockFilesystem filesystem{};
    EXPECT_CALL(filesystem, copy(dummyPath1 / "b.jpg", dummyPath2 / "aaa.jpg"));

    ProcessorConfig config = createProcessorConfigWithOneMatcher(dummyPath1);
    config.matchers[0].actions = {createCopyAction(dummyPath2, "aaa")};
    Processor processor{config, eventQueue, filesystem, nullLogger};

    pushFileCreationEvent(dummyPath1, dummyPath1 / "b.jpg");
    pushInterruptEvent();
    processor.run();
}

TEST_F(ProcessorTest, whenMoveActionIsTriggeredThenRequestFileMove) {
    MockFilesystem filesystem{};
    EXPECT_CALL(filesystem, move(dummyPath1 / "b.jpg", dummyPath2 / "aaa.jpg"));

    ProcessorConfig config = createProcessorConfigWithOneMatcher(dummyPath1);
    config.matchers[0].actions = {createMoveAction(dummyPath2, "aaa")};
    Processor processor{config, eventQueue, filesystem, nullLogger};

    pushFileCreationEvent(dummyPath1, dummyPath1 / "b.jpg");
    pushInterruptEvent();
    processor.run();
}

TEST_F(ProcessorTest, whenRemoveActionIsTriggeredThenRequestFileMove) {
    MockFilesystem filesystem{};
    EXPECT_CALL(filesystem, remove(dummyPath1 / "b.jpg"));

    ProcessorConfig config = createProcessorConfigWithOneMatcher(dummyPath1);
    config.matchers[0].actions = {createRemoveAction()};
    Processor processor{config, eventQueue, filesystem, nullLogger};

    pushFileCreationEvent(dummyPath1, dummyPath1 / "b.jpg");
    pushInterruptEvent();
    processor.run();
}

TEST_F(ProcessorTest, givenCounterUsedWhenCopyActionIsTriggeredThenCheckForFirstFreeFilename) {
    MockFilesystem filesystem{};
    {
        InSequence seq{};
        EXPECT_CALL(filesystem, exists(dummyPath2 / "000.jpg")).WillOnce(Return(false));
        EXPECT_CALL(filesystem, copy(dummyPath1 / "b.jpg", dummyPath2 / "000.jpg"));
    }

    ProcessorConfig config = createProcessorConfigWithOneMatcher(dummyPath1);
    config.matchers[0].actions = {createCopyAction(dummyPath2, "###")};
    Processor processor{config, eventQueue, filesystem, nullLogger};

    pushFileCreationEvent(dummyPath1, dummyPath1 / "b.jpg");
    pushInterruptEvent();
    processor.run();
}

TEST_F(ProcessorTest, givenCounterUsedAndThereAreExistingFilesWhenCopyActionIsTriggeredThenCheckForFirstFreeFilename) {
    MockFilesystem filesystem{};
    {
        InSequence seq{};
        EXPECT_CALL(filesystem, exists(dummyPath2 / "000.jpg")).WillOnce(Return(true));
        EXPECT_CALL(filesystem, exists(dummyPath2 / "001.jpg")).WillOnce(Return(true));
        EXPECT_CALL(filesystem, exists(dummyPath2 / "002.jpg")).WillOnce(Return(true));
        EXPECT_CALL(filesystem, exists(dummyPath2 / "003.jpg")).WillOnce(Return(true));
        EXPECT_CALL(filesystem, exists(dummyPath2 / "004.jpg")).WillOnce(Return(false));
        EXPECT_CALL(filesystem, copy(dummyPath1 / "b.jpg", dummyPath2 / "004.jpg"));
    }

    ProcessorConfig config = createProcessorConfigWithOneMatcher(dummyPath1);
    config.matchers[0].actions = {createCopyAction(dummyPath2, "###")};
    Processor processor{config, eventQueue, filesystem, nullLogger};

    pushFileCreationEvent(dummyPath1, dummyPath1 / "b.jpg");
    pushInterruptEvent();
    processor.run();
}

TEST_F(ProcessorTest, givenNameVariableUsedWhenCopyActionIsTriggeredThenResolveNameProperly) {
    MockFilesystem filesystem{};
    EXPECT_CALL(filesystem, copy(dummyPath1 / "b.jpg", dummyPath2 / "a_b_c.jpg"));

    ProcessorConfig config = createProcessorConfigWithOneMatcher(dummyPath1);
    config.matchers[0].actions = {
        createCopyAction(dummyPath2, "a_${name}_c"),
    };
    Processor processor{config, eventQueue, filesystem, nullLogger};

    pushFileCreationEvent(dummyPath1, dummyPath1 / "b.jpg");
    pushInterruptEvent();
    processor.run();
}

TEST_F(ProcessorTest, givenPreviousNameVariableUsedWhenCopyActionIsTriggeredThenResolveNameProperly) {
    MockFilesystem filesystem{};
    EXPECT_CALL(filesystem, exists(dummyPath2 / "000.jpg"));
    EXPECT_CALL(filesystem, copy(dummyPath1 / "b.jpg", dummyPath2 / "000.jpg"));
    EXPECT_CALL(filesystem, copy(dummyPath1 / "b.jpg", dummyPath2 / "a_000_b.jpg"));

    ProcessorConfig config = createProcessorConfigWithOneMatcher(dummyPath1);
    config.matchers[0].actions = {
        createCopyAction(dummyPath2, "###"),
        createCopyAction(dummyPath2, "a_${previousName}_b"),
    };
    Processor processor{config, eventQueue, filesystem, nullLogger};

    pushFileCreationEvent(dummyPath1, dummyPath1 / "b.jpg");
    pushInterruptEvent();
    processor.run();
}

TEST_F(ProcessorTest, givenExtensionVariableUsedWhenCopyActionIsTriggeredThenResolveNameProperly) {
    MockFilesystem filesystem{};
    EXPECT_CALL(filesystem, copy(dummyPath1 / "b.jpg", dummyPath2 / "a_jpg_c.jpg"));

    ProcessorConfig config = createProcessorConfigWithOneMatcher(dummyPath1);
    config.matchers[0].actions = {createCopyAction(dummyPath2, "a_${extension}_c")};
    Processor processor{config, eventQueue, filesystem, nullLogger};

    pushFileCreationEvent(dummyPath1, dummyPath1 / "b.jpg");
    pushInterruptEvent();
    processor.run();
}

TEST_F(ProcessorTest, givenMultipleVariablesUsedWhenCopyActionIsTriggeredThenResolveNameProperly) {
    MockFilesystem filesystem{};
    EXPECT_CALL(filesystem, copy(dummyPath1 / "b.jpg", dummyPath2 / "dst.jpg"));
    EXPECT_CALL(filesystem, copy(dummyPath1 / "b.jpg", dummyPath2 / "dst_b_jpg_b.jpg"));

    ProcessorConfig config = createProcessorConfigWithOneMatcher(dummyPath1);
    config.matchers[0].actions = {
        createCopyAction(dummyPath2, "dst"),
        createCopyAction(dummyPath2, "${previousName}_${name}_${extension}_${name}"),
    };
    Processor processor{config, eventQueue, filesystem, nullLogger};

    pushFileCreationEvent(dummyPath1, dummyPath1 / "b.jpg");
    pushInterruptEvent();
    processor.run();
}

TEST_F(ProcessorTest, givenEventFromDifferentDirectoryThanWatchedWhenEventIsTriggeredThenDoNotExecuteAnyActions) {
    MockFilesystem filesystem{};
    ProcessorConfig config = createProcessorConfigWithOneMatcher(dummyPath2);
    config.matchers[0].actions = {createCopyAction(dummyPath2, "dst")};
    Processor processor{config, eventQueue, filesystem, nullLogger};

    pushFileCreationEvent(dummyPath1, dummyPath1 / "b.jpg");
    pushInterruptEvent();
    processor.run();
}

TEST_F(ProcessorTest, givenMultipleActionMatchersWhenEventIsTriggeredThenSelectProperActionMatcher) {
    MockFilesystem filesystem{};
    EXPECT_CALL(filesystem, copy(dummyPath1 / "x.jpg", dummyPath1 / "abc.jpg"));
    EXPECT_CALL(filesystem, copy(dummyPath2 / "y.jpg", dummyPath2 / "def.jpg"));

    ProcessorConfig config = createProcessorConfig({dummyPath1, dummyPath2});
    config.matchers[0].actions = {createCopyAction(dummyPath1, "abc")};
    config.matchers[1].actions = {createCopyAction(dummyPath2, "def")};
    Processor processor{config, eventQueue, filesystem, nullLogger};

    pushFileCreationEvent(dummyPath1, dummyPath1 / "x.jpg");
    pushFileCreationEvent(dummyPath2, dummyPath2 / "y.jpg");
    pushInterruptEvent();
    processor.run();
}

TEST_F(ProcessorTest, givenExtensionFiltersNotSatisfiedWhenEventIsTriggeredThenSkipIt) {
    MockFilesystem filesystem{};
    EXPECT_CALL(filesystem, copy(dummyPath1 / "a.jpg", dummyPath2 / "b.jpg"));
    EXPECT_CALL(filesystem, copy(dummyPath1 / "a.png", dummyPath2 / "b.png"));

    ProcessorConfig config = createProcessorConfigWithOneMatcher(dummyPath1);
    config.matchers[0].actions = {createCopyAction(dummyPath2, "b")};
    config.matchers[0].watchedExtensions = {"png", "jpg"};
    Processor processor{config, eventQueue, filesystem, nullLogger};

    pushFileCreationEvent(dummyPath1, dummyPath1 / "a.jpg");
    pushFileCreationEvent(dummyPath1, dummyPath1 / "a.pdf");
    pushFileCreationEvent(dummyPath1, dummyPath1 / "a.png");
    pushFileCreationEvent(dummyPath1, dummyPath1 / "a.pngg");
    pushInterruptEvent();
    processor.run();
}

TEST_F(ProcessorTest, givenMultipleActionMatchersExtensionFiltersNotSatisfiedWhenEventIsTriggeredThenSelectNextMatcher) {
    MockFilesystem filesystem{};
    EXPECT_CALL(filesystem, copy(dummyPath1 / "a.jpg", dummyPath2 / "image.jpg"));
    EXPECT_CALL(filesystem, copy(dummyPath1 / "a.png", dummyPath2 / "image.png"));
    EXPECT_CALL(filesystem, copy(dummyPath1 / "a.mp3", dummyPath2 / "song.mp3"));
    EXPECT_CALL(filesystem, copy(dummyPath1 / "a.pdf", dummyPath2 / "other.pdf"));

    ProcessorConfig config = createProcessorConfig({dummyPath1, dummyPath1, dummyPath1});
    config.matchers[0].actions = {createCopyAction(dummyPath2, "image")};
    config.matchers[0].watchedExtensions = {"png", "jpg"};
    config.matchers[1].actions = {createCopyAction(dummyPath2, "song")};
    config.matchers[1].watchedExtensions = {"mp3"};
    config.matchers[2].actions = {createCopyAction(dummyPath2, "other")};
    config.matchers[2].watchedExtensions = {};
    Processor processor{config, eventQueue, filesystem, nullLogger};

    pushFileCreationEvent(dummyPath1, dummyPath1 / "a.jpg");
    pushFileCreationEvent(dummyPath1, dummyPath1 / "a.png");
    pushFileCreationEvent(dummyPath1, dummyPath1 / "a.mp3");
    pushFileCreationEvent(dummyPath1, dummyPath1 / "a.pdf");
    pushInterruptEvent();
    processor.run();
}

TEST_F(ProcessorTest, whenMoveActionIsExecutedThenLogInfo) {
    NiceMock<MockFilesystem> filesystem{false};
    MockLogger logger{};

    EXPECT_CALL(logger, log(LogLevel::Info, "Processor moving file a/file.txt to b/file.txt"));

    ProcessorConfig config = createProcessorConfigWithOneMatcher("a");
    config.matchers[0].actions = {createMoveAction("b", "${name}")};
    Processor processor{config, eventQueue, filesystem, logger};

    pushFileCreationEvent("a", "a/file.txt");
    pushInterruptEvent();
    processor.run();
}

TEST_F(ProcessorTest, whenCopyActionIsExecutedThenLogInfo) {
    NiceMock<MockFilesystem> filesystem{false};
    MockLogger logger{};

    EXPECT_CALL(logger, log(LogLevel::Info, "Processor copying file a/file.txt to b/file.txt"));

    ProcessorConfig config = createProcessorConfigWithOneMatcher("a");
    config.matchers[0].actions = {createCopyAction("b", "${name}")};
    Processor processor{config, eventQueue, filesystem, logger};

    pushFileCreationEvent("a", "a/file.txt");
    pushInterruptEvent();
    processor.run();
}

TEST_F(ProcessorTest, whenRemoveActionIsExecutedThenLogInfo) {
    NiceMock<MockFilesystem> filesystem{false};
    MockLogger logger{};

    EXPECT_CALL(logger, log(LogLevel::Info, "Processor removing file a/file.txt"));

    ProcessorConfig config = createProcessorConfigWithOneMatcher("a");
    config.matchers[0].actions = {createRemoveAction()};
    Processor processor{config, eventQueue, filesystem, logger};

    pushFileCreationEvent("a", "a/file.txt");
    pushInterruptEvent();
    processor.run();
}

TEST_F(ProcessorTest, givenMultipleActionsWhenTheyAreExecutedThenLogInfo) {
    NiceMock<MockFilesystem> filesystem{false};
    MockLogger logger{};

    EXPECT_CALL(logger, log(LogLevel::Info, "Processor copying file a/file.txt to b/file.txt"));
    EXPECT_CALL(logger, log(LogLevel::Info, "Processor moving file a/file.txt to c/file.txt"));
    EXPECT_CALL(logger, log(LogLevel::Info, "Processor removing file a/file.txt"));

    ProcessorConfig config = createProcessorConfigWithOneMatcher("a");
    config.matchers[0].actions = {
        createCopyAction("b", "${name}"),
        createMoveAction("c", "${name}"),
        createRemoveAction(),
    };
    Processor processor{config, eventQueue, filesystem, logger};

    pushFileCreationEvent("a", "a/file.txt");
    pushInterruptEvent();
    processor.run();
}

TEST_F(ProcessorTest, givenNoMatchingActionMatcherWhenEventIsProcessedThenLogInfo) {
    MockFilesystem filesystem{};
    MockLogger logger{};

    EXPECT_CALL(logger, log(LogLevel::Info, "Processor could not match file a/file.txt to any action matcher"));

    ProcessorConfig config = createProcessorConfigWithOneMatcher("a");
    config.matchers[0].actions = {createCopyAction("b", "${name}")};
    config.matchers[0].watchedExtensions = {"png"};
    Processor processor{config, eventQueue, filesystem, logger};

    pushFileCreationEvent("a", "a/file.txt");
    pushInterruptEvent();
    processor.run();
}

TEST_F(ProcessorTest, givenPrintActionWhenEventIsProcessedThenLogInfo) {
    MockFilesystem filesystem{};
    MockLogger logger{};

    {
        InSequence seq{};
        EXPECT_CALL(logger, log(LogLevel::Info, "File a/file.txt has been created"));
        EXPECT_CALL(logger, log(LogLevel::Info, "File a/file.txt has been removed"));
        EXPECT_CALL(logger, log(LogLevel::Info, "File a/file.txt has been modified"));
        EXPECT_CALL(logger, log(LogLevel::Info, "A file has been moved from path a/file.txt"));
        EXPECT_CALL(logger, log(LogLevel::Info, "A file has been moved to a/file.txt"));
    }

    ProcessorConfig config = createProcessorConfigWithOneMatcher("a");
    config.matchers[0].actions = {createPrintAction()};
    Processor processor{config, eventQueue, filesystem, logger};

    pushFileEvent("a", FileEvent::Type::Add, "a/file.txt");
    pushFileEvent("a", FileEvent::Type::Remove, "a/file.txt");
    pushFileEvent("a", FileEvent::Type::Modify, "a/file.txt");
    pushFileEvent("a", FileEvent::Type::RenameOld, "a/file.txt");
    pushFileEvent("a", FileEvent::Type::RenameNew, "a/file.txt");
    pushInterruptEvent();
    processor.run();
}
