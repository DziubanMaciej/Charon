#include "charon/processor/processor.h"
#include "charon/processor/processor_config.h"
#include "os_tests/test_files_helper.h"

#include <gtest/gtest.h>

struct ProcessorTest : ::testing::Test {
    void SetUp() override {
        srcPath = TestFilesHelper::createDirectory("src");
        dstPath = TestFilesHelper::createDirectory("dst");
    }

    ProcessorAction createCopyAction(const std::string &destinationName) {
        ProcessorAction::MoveOrCopy data{};
        data.destinationDir = dstPath;
        data.destinationName = destinationName;
        return ProcessorAction{ProcessorAction::Type::Copy, data};
    }

    ProcessorAction createMoveAction(const std::string &destinationName) {
        ProcessorAction::MoveOrCopy data{};
        data.destinationDir = dstPath;
        data.destinationName = destinationName;
        return ProcessorAction{ProcessorAction::Type::Move, data};
    }

    ProcessorConfig createProcessorConfigWithOneMatcher() {
        ProcessorActionMatcher matcher{};
        matcher.watchedFolder = srcPath;
        ProcessorConfig config{};
        config.matchers = {matcher};
        return config;
    }

    FileEvent createInterruptEvent() {
        return FileEvent{srcPath, FileEvent::Type::Interrupt, std::filesystem::path{}};
    }

    std::filesystem::path srcPath{};
    std::filesystem::path dstPath{};
    BlockingQueue<FileEvent> eventQueue{};
};

TEST_F(ProcessorTest, givenFileMoveActionTriggeredWhenProcessorIsRunningThenMoveFile) {
    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {createMoveAction("niceFile")};
    Processor processor{config, eventQueue};

    std::thread processorThread{[&]() { processor.run(); }};
    TestFilesHelper::createFile(srcPath / "a");
    eventQueue.push(FileEvent{srcPath, FileEvent::Type::Add, srcPath / "a"});
    eventQueue.push(createInterruptEvent());
    processorThread.join();

    EXPECT_FALSE(TestFilesHelper::fileExists(srcPath / "a"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "niceFile"));
}

TEST_F(ProcessorTest, givenFileCopyActionTriggeredWhenProcessorIsRunningThenCopyFile) {
    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {createCopyAction("niceFile")};
    Processor processor{config, eventQueue};

    std::thread processorThread{[&]() { processor.run(); }};
    TestFilesHelper::createFile(srcPath / "a");
    eventQueue.push(FileEvent{srcPath, FileEvent::Type::Add, srcPath / "a"});
    eventQueue.push(createInterruptEvent());
    processorThread.join();

    EXPECT_TRUE(TestFilesHelper::fileExists(srcPath / "a"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "niceFile"));
}

TEST_F(ProcessorTest, givenFileWithExtensionWhenCopyingFileThenPreserveExtension) {
    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {createCopyAction("niceFile")};
    Processor processor{config, eventQueue};

    std::thread processorThread{[&]() { processor.run(); }};
    TestFilesHelper::createFile(srcPath / "a.jpg");
    eventQueue.push(FileEvent{srcPath, FileEvent::Type::Add, srcPath / "a.jpg"});
    eventQueue.push(createInterruptEvent());
    processorThread.join();

    EXPECT_TRUE(TestFilesHelper::fileExists(srcPath / "a.jpg"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "niceFile.jpg"));
}

TEST_F(ProcessorTest, givenFileWithMultipleExtensionsWhenCopyingFileThenPreserveOnlyLastExtension) {
    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {createCopyAction("niceFile")};
    Processor processor{config, eventQueue};

    std::thread processorThread{[&]() { processor.run(); }};
    TestFilesHelper::createFile(srcPath / "a.gif.pdf.jpg");
    eventQueue.push(FileEvent{srcPath, FileEvent::Type::Add, srcPath / "a.gif.pdf.jpg"});
    eventQueue.push(createInterruptEvent());
    processorThread.join();

    EXPECT_TRUE(TestFilesHelper::fileExists(srcPath / "a.gif.pdf.jpg"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "niceFile.jpg"));
}

TEST_F(ProcessorTest, givenFileCopyActionTriggeredAndNameVariableUsedWhenProcessorIsRunningThenCopyFile) {
    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {createCopyAction("a_${name}_c")};
    Processor processor{config, eventQueue};

    std::thread processorThread{[&]() { processor.run(); }};
    TestFilesHelper::createFile(srcPath / "b");
    eventQueue.push(FileEvent{srcPath, FileEvent::Type::Add, srcPath / "b"});
    eventQueue.push(createInterruptEvent());
    processorThread.join();

    EXPECT_TRUE(TestFilesHelper::fileExists(srcPath / "b"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "a_b_c"));
}

TEST_F(ProcessorTest, givenFileCopyActionTriggeredAndExtensionVariableUsedWhenProcessorIsRunningThenCopyFile) {
    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {createCopyAction("a_${extension}_c")};
    Processor processor{config, eventQueue};

    TestFilesHelper::createFile(srcPath / "b.jpg");
    eventQueue.push(FileEvent{srcPath, FileEvent::Type::Add, srcPath / "b.jpg"});
    eventQueue.push(createInterruptEvent());

    processor.run();

    EXPECT_TRUE(TestFilesHelper::fileExists(srcPath / "b.jpg"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "a_jpg_c.jpg"));
}

TEST_F(ProcessorTest, givenFileCopyActionTriggeredAndCounterIsUsedWhenProcessorIsRunningThenCopyFile) {
    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {createCopyAction("file_###")};
    Processor processor{config, eventQueue};

    std::thread processorThread{[&]() { processor.run(); }};
    TestFilesHelper::createFile(srcPath / "a");
    eventQueue.push(FileEvent{srcPath, FileEvent::Type::Add, srcPath / "a"});
    eventQueue.push(createInterruptEvent());
    processorThread.join();

    EXPECT_TRUE(TestFilesHelper::fileExists(srcPath / "a"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "file_000"));
}
