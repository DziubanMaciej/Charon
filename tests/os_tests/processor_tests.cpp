#include "charon/charon/os_handle.h"
#include "charon/processor/processor.h"
#include "charon/util/filesystem_impl.h"
#include "charon/util/logger.h"
#include "charon/util/time.h"
#include "os_tests/fixtures/processor_config_fixture.h"
#include "os_tests/test_helpers.h"

#include <gtest/gtest.h>
#include <regex>

struct ProcessorTest : ::testing::Test, ProcessorConfigFixture {
    void SetUp() override {
        ProcessorConfigFixture::SetUp();
    }

    void pushFileCreationEventAndCreateFile(const std::filesystem::path &path) {
        TestFilesHelper::createFile(path);
        eventQueue.push(FileEvent{srcPath, FileEvent::Type::Add, path});
    }

    void pushInterruptEvent() {
        eventQueue.push(FileEvent::interruptEvent);
    }

    bool isFileLocked(const std::filesystem::path &path) {
        auto [lockedFileHandle, lockResult] = filesystem.lockFile(path);
        switch (lockResult) {
        case Filesystem::LockResult::DoesNotExist:
            return false;
        case Filesystem::LockResult::Success:
            filesystem.unlockFile(lockedFileHandle);
            return false;
        case Filesystem::LockResult::UsedByOtherProcess:
            return true;
        default:
            GTEST_NONFATAL_FAILURE_("No access to test file");
            FATAL_ERROR("Test error")
        }
    }

    FileEventQueue eventQueue{};
    FilesystemImpl filesystem{};
};

TEST_F(ProcessorTest, givenFileMoveActionTriggeredWhenProcessorIsRunningThenMoveFile) {
    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {createMoveAction("niceFile")};
    Processor processor{config, eventQueue, filesystem};

    pushFileCreationEventAndCreateFile(srcPath / "a");
    pushInterruptEvent();
    processor.run();

    EXPECT_FALSE(TestFilesHelper::fileExists(srcPath / "a"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "niceFile"));
}

TEST_F(ProcessorTest, givenFileCopyActionTriggeredWhenProcessorIsRunningThenCopyFile) {
    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {createCopyAction("niceFile")};
    Processor processor{config, eventQueue, filesystem};

    pushFileCreationEventAndCreateFile(srcPath / "a");
    pushInterruptEvent();
    processor.run();

    EXPECT_TRUE(TestFilesHelper::fileExists(srcPath / "a"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "niceFile"));
}

TEST_F(ProcessorTest, givenFileRemoveActionTriggeredWhenProcessorIsRunningThenRemoveFile) {
    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {createRemoveAction()};
    Processor processor{config, eventQueue, filesystem};

    pushFileCreationEventAndCreateFile(srcPath / "a");
    pushInterruptEvent();
    processor.run();

    EXPECT_FALSE(TestFilesHelper::fileExists(srcPath / "a"));
    EXPECT_FALSE(TestFilesHelper::fileExists(dstPath / "a"));
}

TEST_F(ProcessorTest, givenFileWithExtensionWhenCopyingFileThenPreserveExtension) {
    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {createCopyAction("niceFile")};
    Processor processor{config, eventQueue, filesystem};

    pushFileCreationEventAndCreateFile(srcPath / "a.jpg");
    pushInterruptEvent();
    processor.run();

    EXPECT_TRUE(TestFilesHelper::fileExists(srcPath / "a.jpg"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "niceFile.jpg"));
}

TEST_F(ProcessorTest, givenFileWithMultipleExtensionsWhenCopyingFileThenPreserveOnlyLastExtension) {
    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {createCopyAction("niceFile")};
    Processor processor{config, eventQueue, filesystem};

    pushFileCreationEventAndCreateFile(srcPath / "a.gif.pdf.jpg");
    pushInterruptEvent();
    processor.run();

    EXPECT_TRUE(TestFilesHelper::fileExists(srcPath / "a.gif.pdf.jpg"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "niceFile.jpg"));
}

TEST_F(ProcessorTest, givenFileCopyActionTriggeredAndNameVariableUsedWhenProcessorIsRunningThenCopyFile) {
    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {createCopyAction("a_${name}_c")};
    Processor processor{config, eventQueue, filesystem};

    pushFileCreationEventAndCreateFile(srcPath / "b");
    pushInterruptEvent();
    processor.run();

    EXPECT_TRUE(TestFilesHelper::fileExists(srcPath / "b"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "a_b_c"));
}

TEST_F(ProcessorTest, givenFileCopyActionTriggeredAndExtensionVariableUsedWhenProcessorIsRunningThenCopyFile) {
    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {createCopyAction("a_${extension}_c")};
    Processor processor{config, eventQueue, filesystem};

    pushFileCreationEventAndCreateFile(srcPath / "b.jpg");
    pushInterruptEvent();
    processor.run();

    EXPECT_TRUE(TestFilesHelper::fileExists(srcPath / "b.jpg"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "a_jpg_c.jpg"));
}

TEST_F(ProcessorTest, givenFileCopyActionTriggeredAndPreviousNameVariableUsedWhenProcessorIsRunningThenCopyFile) {
    ProcessorConfig config = createProcessorConfigWithOneMatcher();

    auto dstPath2 = TestFilesHelper::createDirectory("dst2");

    config.matchers[0].actions = {
        createCopyAction("###"),
        createCopyAction("${previousName}", dstPath2),
        createCopyAction("a_${previousName}_b"),
        createCopyAction("a_${previousName}_b"),
    };
    Processor processor{config, eventQueue, filesystem};

    pushFileCreationEventAndCreateFile(srcPath / "a.png");
    pushFileCreationEventAndCreateFile(srcPath / "b.png");
    pushFileCreationEventAndCreateFile(srcPath / "c.png");
    pushInterruptEvent();
    processor.run();

    EXPECT_TRUE(TestFilesHelper::fileExists(srcPath / "a.png"));
    EXPECT_TRUE(TestFilesHelper::fileExists(srcPath / "b.png"));
    EXPECT_TRUE(TestFilesHelper::fileExists(srcPath / "c.png"));

    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "000.png"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "001.png"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "002.png"));

    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath2 / "000.png"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath2 / "001.png"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath2 / "002.png"));

    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "a_000_b.png"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "a_001_b.png"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "a_002_b.png"));

    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "a_a_000_b_b.png"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "a_a_001_b_b.png"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "a_a_002_b_b.png"));
}

TEST_F(ProcessorTest, givenFileCopyActionTriggeredAndCounterIsUsedWhenProcessorIsRunningThenCopyFile) {
    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {createCopyAction("file_###")};
    Processor processor{config, eventQueue, filesystem};

    pushFileCreationEventAndCreateFile(srcPath / "a");
    pushFileCreationEventAndCreateFile(srcPath / "b");
    pushInterruptEvent();
    processor.run();

    EXPECT_TRUE(TestFilesHelper::fileExists(srcPath / "a"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "file_000"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "file_001"));
    EXPECT_FALSE(TestFilesHelper::fileExists(dstPath / "file_002"));
}

TEST_F(ProcessorTest, givenFilesWithMatchingExtensionsTriggeredAndCounterIsUsedWhenProcessorIsRunningThenCopyFile) {
    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {createCopyAction("file_###")};
    Processor processor{config, eventQueue, filesystem};

    pushFileCreationEventAndCreateFile(srcPath / "a.pdf");
    pushFileCreationEventAndCreateFile(srcPath / "b.pdf");
    pushInterruptEvent();
    processor.run();

    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "file_000.pdf"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "file_001.pdf"));
    EXPECT_FALSE(TestFilesHelper::fileExists(dstPath / "file_002"));
}

TEST_F(ProcessorTest, givenFilesWithDifferentExtensionsTriggeredAndCounterIsUsedWhenProcessorIsRunningThenCopyFile) {
    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {createCopyAction("file_###")};
    Processor processor{config, eventQueue, filesystem};

    pushFileCreationEventAndCreateFile(srcPath / "a.pdf");
    pushFileCreationEventAndCreateFile(srcPath / "b.gif");
    pushInterruptEvent();
    processor.run();

    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "file_000.pdf"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "file_001.gif"));
}

TEST_F(ProcessorTest, givenGapInCounterWhenCounterIsUsedThenFillTheGaps) {
    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {createMoveAction("file_###")};
    Processor processor{config, eventQueue, filesystem};

    TestFilesHelper::createFile(dstPath / "file_000.txt");
    TestFilesHelper::createFile(dstPath / "file_001.txt");
    TestFilesHelper::createFile(dstPath / "file_002.txt");
    TestFilesHelper::createFile(dstPath / "file_004.txt");
    TestFilesHelper::createFile(dstPath / "file_007.txt");
    TestFilesHelper::createFile(dstPath / "file_008.txt");

    pushFileCreationEventAndCreateFile(srcPath / "a.1");
    pushFileCreationEventAndCreateFile(srcPath / "a.2");
    pushFileCreationEventAndCreateFile(srcPath / "a.3");
    pushFileCreationEventAndCreateFile(srcPath / "a.4");
    pushFileCreationEventAndCreateFile(srcPath / "a.5");
    pushInterruptEvent();
    processor.run();

    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "file_000.txt"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "file_001.txt"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "file_002.txt"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "file_003.1"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "file_004.txt"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "file_005.2"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "file_006.3"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "file_007.txt"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "file_008.txt"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "file_009.4"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "file_010.5"));

    EXPECT_EQ(0u, TestFilesHelper::countFilesInDirectory(srcPath));
    EXPECT_EQ(11u, TestFilesHelper::countFilesInDirectory(dstPath));
}

TEST_F(ProcessorTest, givenGapOnZeroIndexInCounterWhenCounterIsUsedThenFillTheGap) {
    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {createMoveAction("file_###")};
    Processor processor{config, eventQueue, filesystem};

    TestFilesHelper::createFile(dstPath / "file_001.txt");
    TestFilesHelper::createFile(dstPath / "file_002.txt");
    TestFilesHelper::createFile(dstPath / "file_003.txt");

    pushFileCreationEventAndCreateFile(srcPath / "a.1");
    pushInterruptEvent();
    processor.run();

    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "file_000.1"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "file_001.txt"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "file_002.txt"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "file_003.txt"));

    EXPECT_EQ(0u, TestFilesHelper::countFilesInDirectory(srcPath));
    EXPECT_EQ(4u, TestFilesHelper::countFilesInDirectory(dstPath));
}

TEST_F(ProcessorTest, givenMultipleFileCopyActionsTriggeredAndCounterIsUsedWhenProcessorIsRunningThenCopyFile) {
    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {createCopyAction("file_###")};
    Processor processor{config, eventQueue, filesystem};

    for (int i = 0; i < 16; i++) {
        auto srcFileName = std::string{"srcFile"} + std::to_string(i);
        pushFileCreationEventAndCreateFile(srcPath / srcFileName);
    }
    pushInterruptEvent();
    processor.run();

    for (int i = 0; i < 16; i++) {
        auto srcFileName = std::string{"srcFile"} + std::to_string(i);
        EXPECT_TRUE(TestFilesHelper::fileExists(srcPath / srcFileName));

        std::ostringstream stream{};
        stream << std::setw(3) << std::setfill('0') << i;
        auto dstFileName = std::string{"file_"} + stream.str();

        EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / dstFileName));
    }
}

TEST_F(ProcessorTest, givenMultipleActionsTriggeredAndCounterIsUsedWhenProcessorIsRunningThenCopyFile) {
    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {
        createCopyAction("abc_#"),
        createCopyAction("abc_#"),
        createCopyAction("def"),
        createMoveAction("ghi_${extension}"),
    };
    Processor processor{config, eventQueue, filesystem};

    pushFileCreationEventAndCreateFile(srcPath / "a.png");
    pushInterruptEvent();
    processor.run();

    EXPECT_FALSE(TestFilesHelper::fileExists(srcPath / "a.png"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "abc_0.png"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "abc_1.png"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "def.png"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "ghi_png.png"));
}

TEST_F(ProcessorTest, givenAllFilenamesTakenWhenCounterIsUsedThenReturnError) {
    TimeImpl time{};
    ConsoleLogger consoleLogger{time, defaultLogLevel}; // TODO create MemoryLogger to not use stdout
    auto consoleLoggerSetup = consoleLogger.raiiSetup();

    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {
        createCopyAction("#"),
        createMoveAction("#"),
    };
    Processor processor{config, eventQueue, filesystem};

    for (auto i = 0u; i < 10; i++) {
        TestFilesHelper::createFile(dstPath / std::to_string(i));
    }
    pushFileCreationEventAndCreateFile(srcPath / "10");
    pushInterruptEvent();

    ::testing::internal::CaptureStdout();
    processor.run();

    const std::regex base_regex("\\[[0-9]{2}-[0-9]{2}-[0-9]{4} [0-9]{2}:[0-9]{2}:[0-9]{2}\\]\\[Error\\] Processor could not resolve destination filename.\n"
                                "\\[[0-9]{2}-[0-9]{2}-[0-9]{4} [0-9]{2}:[0-9]{2}:[0-9]{2}\\]\\[Error\\] Processor could not resolve destination filename.\n");
    EXPECT_TRUE(std::regex_match(::testing::internal::GetCapturedStdout(), base_regex));

    EXPECT_TRUE(TestFilesHelper::fileExists(srcPath / "10"));
    EXPECT_FALSE(TestFilesHelper::fileExists(dstPath / "10"));
}

TEST_F(ProcessorTest, givenDestinationDirectoryDoesNotExistWhenCopyOrMoveIsTriggeredThenCreateIt) {
    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {
        createCopyAction("file", dstPath / "a"),
        createMoveAction("file", dstPath / "b"),
    };
    Processor processor{config, eventQueue, filesystem};

    pushFileCreationEventAndCreateFile(srcPath / "file");
    pushInterruptEvent();
    processor.run();

    EXPECT_EQ(0u, TestFilesHelper::countFilesInDirectory(srcPath));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "a" / "file"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "b" / "file"));
}

TEST_F(ProcessorTest, givenDestinationDirectoryDoesNotExistAndCounterIsUsedWhenCopyOrMoveIsTriggeredThenCreateIt) {
    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {
        createCopyAction("file###", dstPath / "a"),
        createMoveAction("file###", dstPath / "b"),
    };
    Processor processor{config, eventQueue, filesystem};

    pushFileCreationEventAndCreateFile(srcPath / "file");
    pushInterruptEvent();
    processor.run();

    EXPECT_EQ(0u, TestFilesHelper::countFilesInDirectory(srcPath));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "a" / "file000"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "b" / "file000"));
}

TEST_F(ProcessorTest, givenEmptyDirectoryCreationEventDoesWhenProcessorIsRunningThenIgnoreIt) {
    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {createMoveAction("${name}", dstPath)};
    Processor processor{config, eventQueue, filesystem};

    auto nonEmptyDir = TestFilesHelper::createDirectory(srcPath / "dir1");
    TestFilesHelper::createFile(nonEmptyDir / "file1");
    TestFilesHelper::createFile(nonEmptyDir / "file2");
    auto emptyDir = TestFilesHelper::createDirectory(srcPath / "dir2");
    eventQueue.push(FileEvent{srcPath, FileEvent::Type::Add, nonEmptyDir});
    eventQueue.push(FileEvent{srcPath, FileEvent::Type::Add, emptyDir});
    pushInterruptEvent();
    processor.run();

    EXPECT_TRUE(TestFilesHelper::fileExists(nonEmptyDir));
    EXPECT_TRUE(TestFilesHelper::fileExists(nonEmptyDir / "file1"));
    EXPECT_TRUE(TestFilesHelper::fileExists(nonEmptyDir / "file2"));
    EXPECT_TRUE(TestFilesHelper::fileExists(emptyDir));
    EXPECT_EQ(0u, TestFilesHelper::countFilesInDirectory(dstPath));
}

TEST_F(ProcessorTest, givenFileRemovedEventWhenProcessorIsRunningThenIgnoreIt) {
    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {createMoveAction("${name}", dstPath)};
    Processor processor{config, eventQueue, filesystem};

    eventQueue.push(FileEvent{srcPath, FileEvent::Type::Remove, srcPath / "removedFile"});
    pushInterruptEvent();
    processor.run();

    EXPECT_EQ(0u, TestFilesHelper::countFilesInDirectory(dstPath));
    EXPECT_EQ(0u, TestFilesHelper::countFilesInDirectory(srcPath));
}

TEST_F(ProcessorTest, givenFileRenamedFromEventWhenProcessorIsRunningThenIgnoreIt) {
    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {createMoveAction("${name}", dstPath)};
    Processor processor{config, eventQueue, filesystem};

    eventQueue.push(FileEvent{srcPath, FileEvent::Type::RenameOld, srcPath / "renamedFromFile"});
    pushInterruptEvent();
    processor.run();

    EXPECT_EQ(0u, TestFilesHelper::countFilesInDirectory(dstPath));
    EXPECT_EQ(0u, TestFilesHelper::countFilesInDirectory(srcPath));
}

TEST_F(ProcessorTest, givenFileModifiedEventWhenProcessorIsRunningThenIgnoreIt) {
    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {createMoveAction("${name}", dstPath)};
    Processor processor{config, eventQueue, filesystem};

    TestFilesHelper::createFile(srcPath / "modifiedFile");
    eventQueue.push(FileEvent{srcPath, FileEvent::Type::Modify, srcPath / "modifiedFile"});
    pushInterruptEvent();
    processor.run();

    EXPECT_TRUE(TestFilesHelper::fileExists(srcPath / "modifiedFile"));
    EXPECT_EQ(0u, TestFilesHelper::countFilesInDirectory(dstPath));
    EXPECT_EQ(1u, TestFilesHelper::countFilesInDirectory(srcPath));
}

TEST_F(ProcessorTest, givenFileRenamedToEventWhenProcessorIsRunningThenPerformAction) {
    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {createMoveAction("${name}", dstPath)};
    Processor processor{config, eventQueue, filesystem};

    TestFilesHelper::createFile(srcPath / "renamedToFile");
    eventQueue.push(FileEvent{srcPath, FileEvent::Type::RenameNew, srcPath / "renamedToFile"});
    pushInterruptEvent();
    processor.run();

    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "renamedToFile"));
    EXPECT_EQ(1u, TestFilesHelper::countFilesInDirectory(dstPath));
    EXPECT_EQ(0u, TestFilesHelper::countFilesInDirectory(srcPath));
}

TEST_F(ProcessorTest, givenFileAlreadyExistsWhenProcessorPerformsMoveActionThenOverwriteIt) {
    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {createMoveAction("conflictingFile", dstPath)};
    Processor processor{config, eventQueue, filesystem};

    TestFilesHelper::createFile(dstPath / "conflictingFile");
    pushFileCreationEventAndCreateFile(srcPath / "myFile");
    pushInterruptEvent();
    processor.run();

    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "conflictingFile"));
    EXPECT_EQ(1u, TestFilesHelper::countFilesInDirectory(dstPath));
    EXPECT_EQ(0u, TestFilesHelper::countFilesInDirectory(srcPath));
}

TEST_F(ProcessorTest, givenFileAlreadyExistsWhenProcessorPerformsCopyActionThenOverwriteIt) {
    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {createCopyAction("conflictingFile", dstPath)};
    Processor processor{config, eventQueue, filesystem};

    TestFilesHelper::createFile(dstPath / "conflictingFile");
    pushFileCreationEventAndCreateFile(srcPath / "myFile");
    pushInterruptEvent();
    processor.run();

    EXPECT_TRUE(TestFilesHelper::fileExists(srcPath / "myFile"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "conflictingFile"));
    EXPECT_EQ(1u, TestFilesHelper::countFilesInDirectory(dstPath));
    EXPECT_EQ(1u, TestFilesHelper::countFilesInDirectory(srcPath));
}

TEST_F(ProcessorTest, givenLockedFileWhenProcessingEventThenUnlockFileAndProcessAction) {
    REQUIRE_FILE_LOCKING_OR_SKIP(filesystem);

    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {createMoveAction("niceFile")};
    Processor processor{config, eventQueue, filesystem};

    auto filePath = srcPath / "a";
    {
        TestFilesHelper::createFile(filePath);

        auto [lockedFileHandle, lockResult] = filesystem.lockFile(filePath);
        ASSERT_EQ(lockResult, Filesystem::LockResult::Success);
        ASSERT_NE(lockedFileHandle, defaultOsHandle);

        eventQueue.push(FileEvent{srcPath, FileEvent::Type::Add, filePath, lockedFileHandle});
        pushInterruptEvent();
    }

    processor.run();

    EXPECT_FALSE(TestFilesHelper::fileExists(srcPath / "a"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "niceFile"));
    EXPECT_FALSE(isFileLocked(filePath));
}

TEST_F(ProcessorTest, givenLockedFileWithIgnoredExtensionWhenProcessingEventThenUnlockFileAndSkipAction) {
    REQUIRE_FILE_LOCKING_OR_SKIP(filesystem);

    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {createMoveAction("niceFile")};
    config.matchers[0].watchedExtensions = {"png"};
    Processor processor{config, eventQueue, filesystem};

    auto filePath = srcPath / "a";
    {
        TestFilesHelper::createFile(filePath);

        auto [lockedFileHandle, lockResult] = filesystem.lockFile(filePath);
        ASSERT_EQ(lockResult, Filesystem::LockResult::Success);
        ASSERT_NE(lockedFileHandle, defaultOsHandle);

        eventQueue.push(FileEvent{srcPath, FileEvent::Type::Add, filePath, lockedFileHandle});
        pushInterruptEvent();
    }

    processor.run();

    EXPECT_TRUE(TestFilesHelper::fileExists(srcPath / "a"));
    EXPECT_EQ(1u, TestFilesHelper::countFilesInDirectory(srcPath));
    EXPECT_EQ(0u, TestFilesHelper::countFilesInDirectory(dstPath));
    EXPECT_FALSE(isFileLocked(filePath));
}
