#include "charon/processor/processor.h"
#include "charon/processor/processor_config.h"
#include "charon/util/logger.h"
#include "unit_tests/fixtures/processor_config_fixture.h"
#include "unit_tests/mocks/mock_filesystem.h"
#include "unit_tests/mocks/mock_logger.h"
#include "unit_tests/mocks/mock_os_handle.h"

#include <gtest/gtest.h>

using ::testing::_;
using ::testing::AtLeast;
using ::testing::InSequence;
using ::testing::Return;
using ::testing::SetArgReferee;

struct ProcessorFixture : ProcessorConfigFixture {
    void pushFileCreationEvent(const std::filesystem::path &watchedDir, const std::filesystem::path &path) {
        pushFileEvent(watchedDir, FileEvent::Type::Add, path);
    }

    void pushFileEvent(const std::filesystem::path &watchedDir, FileEvent::Type type, const std::filesystem::path &path) {
        eventQueue.push(FileEvent{watchedDir, type, path});
    }

    void pushInterruptEvent() {
        eventQueue.push(FileEvent::interruptEvent);
    }

    FileEventQueue eventQueue{};
    NullLogger nullLogger{};
};

struct ProcessorTest : ::testing::Test, ProcessorFixture {};

TEST_F(ProcessorTest, whenCopyActionIsTriggeredThenRequestFileCopy) {
    MockFilesystem filesystem{};
    EXPECT_CALL(filesystem, copy(dummyPath1 / "b.jpg", dummyPath2 / "aaa.jpg"));

    ProcessorConfig config = createProcessorConfigWithOneMatcher(dummyPath1);
    config.matchers[0].actions = {createCopyAction(dummyPath2, "aaa")};
    Processor processor{config, eventQueue, filesystem};

    pushFileCreationEvent(dummyPath1, dummyPath1 / "b.jpg");
    pushInterruptEvent();
    processor.run();
}

TEST_F(ProcessorTest, whenMoveActionIsTriggeredThenRequestFileMove) {
    MockFilesystem filesystem{};
    EXPECT_CALL(filesystem, move(dummyPath1 / "b.jpg", dummyPath2 / "aaa.jpg"));

    ProcessorConfig config = createProcessorConfigWithOneMatcher(dummyPath1);
    config.matchers[0].actions = {createMoveAction(dummyPath2, "aaa")};
    Processor processor{config, eventQueue, filesystem};

    pushFileCreationEvent(dummyPath1, dummyPath1 / "b.jpg");
    pushInterruptEvent();
    processor.run();
}

TEST_F(ProcessorTest, whenRemoveActionIsTriggeredThenRequestFileMove) {
    MockFilesystem filesystem{};
    EXPECT_CALL(filesystem, remove(dummyPath1 / "b.jpg"));

    ProcessorConfig config = createProcessorConfigWithOneMatcher(dummyPath1);
    config.matchers[0].actions = {createRemoveAction()};
    Processor processor{config, eventQueue, filesystem};

    pushFileCreationEvent(dummyPath1, dummyPath1 / "b.jpg");
    pushInterruptEvent();
    processor.run();
}

TEST_F(ProcessorTest, givenCounterUsedWhenCopyActionIsTriggeredThenCheckForFirstFreeFilename) {
    MockFilesystem filesystem{};
    {
        InSequence seq{};
        EXPECT_CALL(filesystem, listFiles(dummyPath2))
            .WillOnce(Return(std::vector<fs::path>{}));
        EXPECT_CALL(filesystem, copy(dummyPath1 / "b.jpg", dummyPath2 / "000.jpg"));
    }

    ProcessorConfig config = createProcessorConfigWithOneMatcher(dummyPath1);
    config.matchers[0].actions = {createCopyAction(dummyPath2, "###")};
    Processor processor{config, eventQueue, filesystem};

    pushFileCreationEvent(dummyPath1, dummyPath1 / "b.jpg");
    pushInterruptEvent();
    processor.run();
}

TEST_F(ProcessorTest, givenCounterUsedAndThereAreExistingFilesWhenCopyActionIsTriggeredThenCheckForFirstFreeFilename) {
    MockFilesystem filesystem{};
    {
        InSequence seq{};
        EXPECT_CALL(filesystem, listFiles(dummyPath2))
            .WillOnce(Return(std::vector<fs::path>{
                dummyPath2 / "000.jpg",
                dummyPath2 / "001.jpg",
                dummyPath2 / "002.jpg",
                dummyPath2 / "003.jpg"}));
        EXPECT_CALL(filesystem, copy(dummyPath1 / "b.jpg", dummyPath2 / "004.jpg"));
    }

    ProcessorConfig config = createProcessorConfigWithOneMatcher(dummyPath1);
    config.matchers[0].actions = {createCopyAction(dummyPath2, "###")};
    Processor processor{config, eventQueue, filesystem};

    pushFileCreationEvent(dummyPath1, dummyPath1 / "b.jpg");
    pushInterruptEvent();
    processor.run();
}

TEST_F(ProcessorTest, givenCounterUsedAndIndexZeroIsFreeWhenCopyActionIsTriggeredThenUseIndexZero) {
    MockFilesystem filesystem{};
    {
        InSequence seq{};
        EXPECT_CALL(filesystem, listFiles(dummyPath2))
            .WillOnce(Return(std::vector<fs::path>{
                dummyPath2 / "001.jpg",
                dummyPath2 / "002.jpg",
                dummyPath2 / "003.jpg"}));
        EXPECT_CALL(filesystem, copy(dummyPath1 / "b.jpg", dummyPath2 / "000.jpg"));
    }

    ProcessorConfig config = createProcessorConfigWithOneMatcher(dummyPath1);
    config.matchers[0].actions = {createCopyAction(dummyPath2, "###")};
    Processor processor{config, eventQueue, filesystem};

    pushFileCreationEvent(dummyPath1, dummyPath1 / "b.jpg");
    pushInterruptEvent();
    processor.run();
}

TEST_F(ProcessorTest, givenCounterUsedAndIndexOneIsFreeWhenCopyActionIsTriggeredThenUseIndexOne) {
    MockFilesystem filesystem{};
    {
        InSequence seq{};
        EXPECT_CALL(filesystem, listFiles(dummyPath2))
            .WillOnce(Return(std::vector<fs::path>{
                dummyPath2 / "000.jpg",
                dummyPath2 / "002.jpg",
                dummyPath2 / "003.jpg"}));
        EXPECT_CALL(filesystem, copy(dummyPath1 / "b.jpg", dummyPath2 / "001.jpg"));
    }

    ProcessorConfig config = createProcessorConfigWithOneMatcher(dummyPath1);
    config.matchers[0].actions = {createCopyAction(dummyPath2, "###")};
    Processor processor{config, eventQueue, filesystem};

    pushFileCreationEvent(dummyPath1, dummyPath1 / "b.jpg");
    pushInterruptEvent();
    processor.run();
}

TEST_F(ProcessorTest, givenCounterUsedAndMultipleGapsInNamesWhenCopyActionsAreTriggeredThenFillGaps) {
    MockFilesystem filesystem{};
    {
        InSequence seq{};
        EXPECT_CALL(filesystem, listFiles(dummyPath2))
            .WillOnce(Return(std::vector<fs::path>{
                dummyPath2 / "000.jpg",
                dummyPath2 / "002.jpg",
                dummyPath2 / "005.jpg",
            }));
        EXPECT_CALL(filesystem, copy(dummyPath1 / "a.jpg", dummyPath2 / "001.jpg"));
        EXPECT_CALL(filesystem, listFiles(dummyPath2))
            .WillOnce(Return(std::vector<fs::path>{
                dummyPath2 / "000.jpg",
                dummyPath2 / "001.jpg",
                dummyPath2 / "002.jpg",
                dummyPath2 / "005.jpg",
            }));
        EXPECT_CALL(filesystem, copy(dummyPath1 / "b.jpg", dummyPath2 / "003.jpg"));
        EXPECT_CALL(filesystem, listFiles(dummyPath2))
            .WillOnce(Return(std::vector<fs::path>{
                dummyPath2 / "000.jpg",
                dummyPath2 / "001.jpg",
                dummyPath2 / "002.jpg",
                dummyPath2 / "003.jpg",
                dummyPath2 / "005.jpg",
            }));
        EXPECT_CALL(filesystem, copy(dummyPath1 / "c.jpg", dummyPath2 / "004.jpg"));
        EXPECT_CALL(filesystem, listFiles(dummyPath2))
            .WillOnce(Return(std::vector<fs::path>{
                dummyPath2 / "000.jpg",
                dummyPath2 / "001.jpg",
                dummyPath2 / "002.jpg",
                dummyPath2 / "003.jpg",
                dummyPath2 / "004.jpg",
                dummyPath2 / "005.jpg",
            }));
        EXPECT_CALL(filesystem, copy(dummyPath1 / "d.jpg", dummyPath2 / "006.jpg"));
    }

    ProcessorConfig config = createProcessorConfigWithOneMatcher(dummyPath1);
    config.matchers[0].actions = {createCopyAction(dummyPath2, "###")};
    Processor processor{config, eventQueue, filesystem};

    pushFileCreationEvent(dummyPath1, dummyPath1 / "a.jpg");
    pushFileCreationEvent(dummyPath1, dummyPath1 / "b.jpg");
    pushFileCreationEvent(dummyPath1, dummyPath1 / "c.jpg");
    pushFileCreationEvent(dummyPath1, dummyPath1 / "d.jpg");
    pushInterruptEvent();
    processor.run();
}

TEST_F(ProcessorTest, givenCounterUsedAndMultipleGapsInNamesWithDifferentExtensionsWhenCopyActionsAreTriggeredThenFillGaps) {
    MockFilesystem filesystem{};
    {
        InSequence seq{};
        EXPECT_CALL(filesystem, listFiles(dummyPath2))
            .WillOnce(Return(std::vector<fs::path>{
                dummyPath2 / "000.jpg",
                dummyPath2 / "002.jpg",
                dummyPath2 / "005.jpg",
            }));
        EXPECT_CALL(filesystem, copy(dummyPath1 / "a.1", dummyPath2 / "001.1"));
        EXPECT_CALL(filesystem, listFiles(dummyPath2))
            .WillOnce(Return(std::vector<fs::path>{
                dummyPath2 / "000.jpg",
                dummyPath2 / "001.1",
                dummyPath2 / "002.jpg",
                dummyPath2 / "005.jpg",
            }));
        EXPECT_CALL(filesystem, copy(dummyPath1 / "b.2", dummyPath2 / "003.2"));
        EXPECT_CALL(filesystem, listFiles(dummyPath2))
            .WillOnce(Return(std::vector<fs::path>{
                dummyPath2 / "000.jpg",
                dummyPath2 / "001.1",
                dummyPath2 / "002.jpg",
                dummyPath2 / "003.2",
                dummyPath2 / "005.jpg",
            }));
        EXPECT_CALL(filesystem, copy(dummyPath1 / "c.3", dummyPath2 / "004.3"));
        EXPECT_CALL(filesystem, listFiles(dummyPath2))
            .WillOnce(Return(std::vector<fs::path>{
                dummyPath2 / "000.jpg",
                dummyPath2 / "001.1",
                dummyPath2 / "002.jpg",
                dummyPath2 / "003.2",
                dummyPath2 / "004.3",
                dummyPath2 / "005.jpg",
            }));
        EXPECT_CALL(filesystem, copy(dummyPath1 / "d.4", dummyPath2 / "006.4"));
    }

    ProcessorConfig config = createProcessorConfigWithOneMatcher(dummyPath1);
    config.matchers[0].actions = {createCopyAction(dummyPath2, "###")};
    Processor processor{config, eventQueue, filesystem};

    pushFileCreationEvent(dummyPath1, dummyPath1 / "a.1");
    pushFileCreationEvent(dummyPath1, dummyPath1 / "b.2");
    pushFileCreationEvent(dummyPath1, dummyPath1 / "c.3");
    pushFileCreationEvent(dummyPath1, dummyPath1 / "d.4");
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
    Processor processor{config, eventQueue, filesystem};

    pushFileCreationEvent(dummyPath1, dummyPath1 / "b.jpg");
    pushInterruptEvent();
    processor.run();
}

TEST_F(ProcessorTest, givenPreviousNameVariableUsedWhenCopyActionIsTriggeredThenResolveNameProperly) {
    MockFilesystem filesystem{};
    EXPECT_CALL(filesystem, listFiles(dummyPath2));
    EXPECT_CALL(filesystem, copy(dummyPath1 / "b.jpg", dummyPath2 / "000.jpg"));
    EXPECT_CALL(filesystem, copy(dummyPath1 / "b.jpg", dummyPath2 / "a_000_b.jpg"));

    ProcessorConfig config = createProcessorConfigWithOneMatcher(dummyPath1);
    config.matchers[0].actions = {
        createCopyAction(dummyPath2, "###"),
        createCopyAction(dummyPath2, "a_${previousName}_b"),
    };
    Processor processor{config, eventQueue, filesystem};

    pushFileCreationEvent(dummyPath1, dummyPath1 / "b.jpg");
    pushInterruptEvent();
    processor.run();
}

TEST_F(ProcessorTest, givenExtensionVariableUsedWhenCopyActionIsTriggeredThenResolveNameProperly) {
    MockFilesystem filesystem{};
    EXPECT_CALL(filesystem, copy(dummyPath1 / "b.jpg", dummyPath2 / "a_jpg_c.jpg"));

    ProcessorConfig config = createProcessorConfigWithOneMatcher(dummyPath1);
    config.matchers[0].actions = {createCopyAction(dummyPath2, "a_${extension}_c")};
    Processor processor{config, eventQueue, filesystem};

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
    Processor processor{config, eventQueue, filesystem};

    pushFileCreationEvent(dummyPath1, dummyPath1 / "b.jpg");
    pushInterruptEvent();
    processor.run();
}

TEST_F(ProcessorTest, givenEventFromDifferentDirectoryThanWatchedWhenEventIsTriggeredThenDoNotExecuteAnyActions) {
    MockFilesystem filesystem{};
    ProcessorConfig config = createProcessorConfigWithOneMatcher(dummyPath2);
    config.matchers[0].actions = {createCopyAction(dummyPath2, "dst")};
    Processor processor{config, eventQueue, filesystem};

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
    Processor processor{config, eventQueue, filesystem};

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
    Processor processor{config, eventQueue, filesystem};

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
    Processor processor{config, eventQueue, filesystem};

    pushFileCreationEvent(dummyPath1, dummyPath1 / "a.jpg");
    pushFileCreationEvent(dummyPath1, dummyPath1 / "a.png");
    pushFileCreationEvent(dummyPath1, dummyPath1 / "a.mp3");
    pushFileCreationEvent(dummyPath1, dummyPath1 / "a.pdf");
    pushInterruptEvent();
    processor.run();
}

TEST_F(ProcessorTest, whenMoveActionIsExecutedThenLogInfo) {
    MockFilesystem filesystem{false};
    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();

    EXPECT_CALL(logger, log(LogLevel::Info, "Processor moving file a/file.txt to b/file.txt"));

    ProcessorConfig config = createProcessorConfigWithOneMatcher("a");
    config.matchers[0].actions = {createMoveAction("b", "${name}")};
    Processor processor{config, eventQueue, filesystem};

    pushFileCreationEvent("a", "a/file.txt");
    pushInterruptEvent();
    processor.run();
}

TEST_F(ProcessorTest, whenCopyActionIsExecutedThenLogInfo) {
    MockFilesystem filesystem{false};
    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();

    EXPECT_CALL(logger, log(LogLevel::Info, "Processor copying file a/file.txt to b/file.txt"));

    ProcessorConfig config = createProcessorConfigWithOneMatcher("a");
    config.matchers[0].actions = {createCopyAction("b", "${name}")};
    Processor processor{config, eventQueue, filesystem};

    pushFileCreationEvent("a", "a/file.txt");
    pushInterruptEvent();
    processor.run();
}

TEST_F(ProcessorTest, whenRemoveActionIsExecutedThenLogInfo) {
    MockFilesystem filesystem{false};
    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();

    EXPECT_CALL(logger, log(LogLevel::Info, "Processor removing file a/file.txt"));

    ProcessorConfig config = createProcessorConfigWithOneMatcher("a");
    config.matchers[0].actions = {createRemoveAction()};
    Processor processor{config, eventQueue, filesystem};

    pushFileCreationEvent("a", "a/file.txt");
    pushInterruptEvent();
    processor.run();
}

TEST_F(ProcessorTest, givenMultipleActionsWhenTheyAreExecutedThenLogInfo) {
    MockFilesystem filesystem{false};
    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();

    EXPECT_CALL(logger, log(LogLevel::Info, "Processor copying file a/file.txt to b/file.txt"));
    EXPECT_CALL(logger, log(LogLevel::Info, "Processor moving file a/file.txt to c/file.txt"));
    EXPECT_CALL(logger, log(LogLevel::Info, "Processor removing file a/file.txt"));

    ProcessorConfig config = createProcessorConfigWithOneMatcher("a");
    config.matchers[0].actions = {
        createCopyAction("b", "${name}"),
        createMoveAction("c", "${name}"),
        createRemoveAction(),
    };
    Processor processor{config, eventQueue, filesystem};

    pushFileCreationEvent("a", "a/file.txt");
    pushInterruptEvent();
    processor.run();
}

TEST_F(ProcessorTest, givenNoMatchingActionMatcherWhenEventIsProcessedThenLogInfo) {
    MockFilesystem filesystem{};
    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();

    EXPECT_CALL(logger, log(LogLevel::Info, "Processor could not match file a/file.txt to any action matcher"));

    ProcessorConfig config = createProcessorConfigWithOneMatcher("a");
    config.matchers[0].actions = {createCopyAction("b", "${name}")};
    config.matchers[0].watchedExtensions = {"png"};
    Processor processor{config, eventQueue, filesystem};

    pushFileCreationEvent("a", "a/file.txt");
    pushInterruptEvent();
    processor.run();
}

TEST_F(ProcessorTest, givenPrintActionWhenEventIsProcessedThenLogInfo) {
    MockFilesystem filesystem{};
    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();

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
    Processor processor{config, eventQueue, filesystem};

    pushFileEvent("a", FileEvent::Type::Add, "a/file.txt");
    pushFileEvent("a", FileEvent::Type::Remove, "a/file.txt");
    pushFileEvent("a", FileEvent::Type::Modify, "a/file.txt");
    pushFileEvent("a", FileEvent::Type::RenameOld, "a/file.txt");
    pushFileEvent("a", FileEvent::Type::RenameNew, "a/file.txt");
    pushInterruptEvent();
    processor.run();
}

TEST_F(ProcessorTest, givenCreateDirectoryEventWhenProcessorIsRunningThenSkipTheEvent) {
    MockFilesystem filesystem{};
    MockLogger logger{false};
    auto loggerSetup = logger.raiiSetup();

    EXPECT_CALL(filesystem, isDirectory(dummyPath1 / "file"))
        .Times(1)
        .WillOnce(Return(false));
    EXPECT_CALL(filesystem, isDirectory(dummyPath1 / "directory"))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(filesystem, copy(dummyPath1 / "file", dummyPath2 / "file"));

    ProcessorConfig config = createProcessorConfigWithOneMatcher("a");
    config.matchers[0].actions = {createCopyAction(dummyPath2, "${name}")};
    Processor processor{config, eventQueue, filesystem};

    pushFileEvent("a", FileEvent::Type::Add, dummyPath1 / "file");
    pushFileEvent("a", FileEvent::Type::Add, dummyPath1 / "directory");
    pushInterruptEvent();
    processor.run();
}

TEST_F(ProcessorTest, adasd) {
    MockFilesystem filesystem{};
    auto lockedFileHandle = mockOsHandle;

    EXPECT_CALL(filesystem, unlockFile(lockedFileHandle))
        .WillOnce(SetArgReferee<0>(defaultOsHandle));
    EXPECT_CALL(filesystem, copy(dummyPath1 / "locked", dummyPath2 / "dst"));
    EXPECT_CALL(filesystem, copy(dummyPath1 / "unlocked", dummyPath2 / "dst"));

    ProcessorConfig config = createProcessorConfigWithOneMatcher(dummyPath1);
    config.matchers[0].actions = {createCopyAction(dummyPath2, "dst")};
    Processor processor{config, eventQueue, filesystem};

    eventQueue.push(FileEvent{dummyPath1, FileEvent::Type::Add, dummyPath1 / "locked", lockedFileHandle});
    eventQueue.push(FileEvent{dummyPath1, FileEvent::Type::Add, dummyPath1 / "unlocked", defaultOsHandle});
    pushInterruptEvent();

    processor.run();
}

struct ProcessorTestWithDifferentEventsAndActions
    : ::testing::TestWithParam<std::tuple<FileEvent::Type, ProcessorAction::Type, bool>>,
      ProcessorFixture {
    ProcessorAction createActionOfType(ProcessorAction::Type type) {
        switch (type) {
        case ProcessorAction::Type::Copy:
            return createCopyAction(dummyPath2, "${name}");
        case ProcessorAction::Type::Move:
            return createMoveAction(dummyPath2, "${name}");
        case ProcessorAction::Type::Remove:
            return createRemoveAction();
        case ProcessorAction::Type::Print:
            return createPrintAction();
        default:
            EXPECT_FALSE(true);
            throw "error";
        }
    }

    void expectActionPerformed(ProcessorAction::Type type, MockFilesystem &filesystem, MockLogger &logger) {
        switch (type) {
        case ProcessorAction::Type::Copy:
            EXPECT_CALL(logger, log).Times(AnyNumber());
            EXPECT_CALL(filesystem, copy);
            break;
        case ProcessorAction::Type::Move:
            EXPECT_CALL(logger, log).Times(AnyNumber());
            EXPECT_CALL(filesystem, move);
            break;
        case ProcessorAction::Type::Remove:
            EXPECT_CALL(logger, log).Times(AnyNumber());
            EXPECT_CALL(filesystem, remove);
            break;
        case ProcessorAction::Type::Print:
            EXPECT_CALL(logger, log);
            break;
        default:
            EXPECT_FALSE(true);
            throw "error";
        }
    }
};

TEST_P(ProcessorTestWithDifferentEventsAndActions, givenNonNewEventAndFilesystemActionWhenProcessorIsRunningThenSkipTheAction) {
    const auto eventType = std::get<FileEvent::Type>(GetParam());
    const auto actionType = std::get<ProcessorAction::Type>(GetParam());
    const auto shouldExecuteAction = std::get<bool>(GetParam());

    MockFilesystem filesystem{};
    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();

    if (shouldExecuteAction) {
        expectActionPerformed(actionType, filesystem, logger);
    }

    ProcessorConfig config = createProcessorConfigWithOneMatcher(dummyPath1);
    config.matchers[0].actions = {createActionOfType(actionType)};
    Processor processor{config, eventQueue, filesystem};
    pushFileEvent(dummyPath1, eventType, dummyPath1 / "file");
    pushInterruptEvent();
    processor.run();
}

static const auto filesystemActions = ::testing::Values(ProcessorAction::Type::Copy,
                                                        ProcessorAction::Type::Move,
                                                        ProcessorAction::Type::Remove);
static const auto nonFilesystemActions = ::testing::Values(ProcessorAction::Type::Print);
static const auto newFileEvents = ::testing::Values(FileEvent::Type::Add,
                                                    FileEvent::Type::RenameNew);
static const auto nonNewFileEvents = ::testing::Values(FileEvent::Type::Modify,
                                                       FileEvent::Type::RenameOld,
                                                       FileEvent::Type::Remove);
static const auto executeAction = ::testing::Values(true);
static const auto doNotExecuteAction = ::testing::Values(false);

INSTANTIATE_TEST_SUITE_P(
    ProcessorTestWithNewFileEventsAndFilesystemActions,
    ProcessorTestWithDifferentEventsAndActions,
    ::testing::Combine(newFileEvents, filesystemActions, executeAction));

INSTANTIATE_TEST_SUITE_P(
    ProcessorTestWithNewFileEventsAndNonFilesystemActions,
    ProcessorTestWithDifferentEventsAndActions,
    ::testing::Combine(newFileEvents, nonFilesystemActions, executeAction));

INSTANTIATE_TEST_SUITE_P(
    ProcessorTestWithNonNewFileEventsAndFilesystemActions,
    ProcessorTestWithDifferentEventsAndActions,
    ::testing::Combine(nonNewFileEvents, filesystemActions, doNotExecuteAction));

INSTANTIATE_TEST_SUITE_P(
    ProcessorTestWithNonNewFileEventsAndNonFilesystemActions,
    ProcessorTestWithDifferentEventsAndActions,
    ::testing::Combine(nonNewFileEvents, nonFilesystemActions, executeAction));

TEST_F(ProcessorTest, ddd_givenCrossDeviceLinkErrorWhenMoveOperationIsTriggerredThenFallbackToCopyPlusRemove) {
    MockFilesystem filesystem{};
    EXPECT_CALL(filesystem, move(fs::path("a/src"), fs::path("b/dst"))).WillOnce(Return(std::make_error_code(std::errc::cross_device_link)));
    EXPECT_CALL(filesystem, copy(fs::path("a/src"), fs::path("b/dst")));
    EXPECT_CALL(filesystem, remove(fs::path("a/src")));

    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();
    EXPECT_CALL(logger, log(LogLevel::Info, "Processor moving file a/src to b/dst"));
    EXPECT_CALL(logger, log(LogLevel::Info, "Move operation had to be emulated with copy+remove."));

    ProcessorConfig config = createProcessorConfigWithOneMatcher("a");
    config.matchers[0].actions = {createMoveAction("b", "dst")};
    Processor processor{config, eventQueue, filesystem};

    pushFileCreationEvent("a", "a/src");
    pushInterruptEvent();
    processor.run();
}

TEST_F(ProcessorTest, ddd_givenCopyOperationFailsWhenPerformingCopyPlusRemoveFallbackThenReportError) {
    const std::error_code err = std::make_error_code(std::errc::illegal_byte_sequence);

    MockFilesystem filesystem{};
    EXPECT_CALL(filesystem, move(fs::path("a/src"), fs::path("b/dst"))).WillOnce(Return(std::make_error_code(std::errc::cross_device_link)));
    EXPECT_CALL(filesystem, copy(fs::path("a/src"), fs::path("b/dst"))).WillOnce(Return(err));

    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();
    EXPECT_CALL(logger, log(LogLevel::Info, "Processor moving file a/src to b/dst"));
    std::ostringstream errorStringStream{};
    errorStringStream << "Filesystem operation returned code " << err.value() << ": " << err.message();
    EXPECT_CALL(logger, log(LogLevel::Error, errorStringStream.str().c_str()));

    ProcessorConfig config = createProcessorConfigWithOneMatcher("a");
    config.matchers[0].actions = {createMoveAction("b", "dst")};
    Processor processor{config, eventQueue, filesystem};

    pushFileCreationEvent("a", "a/src");
    pushInterruptEvent();
    processor.run();
}

TEST_F(ProcessorTest, ddd_givenRemoveOperationFailsWhenPerformingCopyPlusRemoveFallbackThenReportError) {
    const std::error_code err = std::make_error_code(std::errc::permission_denied);

    MockFilesystem filesystem{};
    EXPECT_CALL(filesystem, move(fs::path("a/src"), fs::path("b/dst"))).WillOnce(Return(std::make_error_code(std::errc::cross_device_link)));
    EXPECT_CALL(filesystem, copy(fs::path("a/src"), fs::path("b/dst")));
    EXPECT_CALL(filesystem, remove(fs::path("a/src"))).WillOnce(Return(err));

    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();
    EXPECT_CALL(logger, log(LogLevel::Info, "Processor moving file a/src to b/dst"));
    EXPECT_CALL(logger, log(LogLevel::Error, "Move operation had to be emulated with copy+remove. Remove operation failed"));
    std::ostringstream errorStringStream{};
    errorStringStream << "Filesystem operation returned code " << err.value() << ": " << err.message();
    EXPECT_CALL(logger, log(LogLevel::Error, errorStringStream.str().c_str()));

    ProcessorConfig config = createProcessorConfigWithOneMatcher("a");
    config.matchers[0].actions = {createMoveAction("b", "dst")};
    Processor processor{config, eventQueue, filesystem};

    pushFileCreationEvent("a", "a/src");
    pushInterruptEvent();
    processor.run();
}
