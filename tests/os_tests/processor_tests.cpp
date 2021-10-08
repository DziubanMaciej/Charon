#include "charon/processor/processor.h"
#include "charon/processor/processor_config.h"
#include "charon/util/filesystem_impl.h"
#include "charon/util/logger.h"
#include "os_tests/test_files_helper.h"

#include <gtest/gtest.h>

struct ProcessorTest : ::testing::Test {
    void SetUp() override {
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

    void pushFileCreationEventAndCreateFile(const std::filesystem::path &path) {
        TestFilesHelper::createFile(path);
        eventQueue.push(FileEvent{srcPath, FileEvent::Type::Add, path});
    }

    void pushInterruptEvent() {
        eventQueue.push(FileEvent{srcPath, FileEvent::Type::Interrupt, std::filesystem::path{}});
    }

    std::filesystem::path srcPath{};
    std::filesystem::path dstPath{};
    FileEventQueue eventQueue{};
    NullLogger nullLogger{};
    FilesystemImpl filesystem{};
};

TEST_F(ProcessorTest, givenFileMoveActionTriggeredWhenProcessorIsRunningThenMoveFile) {
    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {createMoveAction("niceFile")};
    Processor processor{config, eventQueue, filesystem, &nullLogger};

    pushFileCreationEventAndCreateFile(srcPath / "a");
    pushInterruptEvent();
    processor.run();

    EXPECT_FALSE(TestFilesHelper::fileExists(srcPath / "a"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "niceFile"));
}

TEST_F(ProcessorTest, givenFileCopyActionTriggeredWhenProcessorIsRunningThenCopyFile) {
    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {createCopyAction("niceFile")};
    Processor processor{config, eventQueue, filesystem, &nullLogger};

    pushFileCreationEventAndCreateFile(srcPath / "a");
    pushInterruptEvent();
    processor.run();

    EXPECT_TRUE(TestFilesHelper::fileExists(srcPath / "a"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "niceFile"));
}

TEST_F(ProcessorTest, givenFileRemoveActionTriggeredWhenProcessorIsRunningThenRemoveFile) {
    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {createRemoveAction()};
    Processor processor{config, eventQueue, filesystem, &nullLogger};

    pushFileCreationEventAndCreateFile(srcPath / "a");
    pushInterruptEvent();
    processor.run();

    EXPECT_FALSE(TestFilesHelper::fileExists(srcPath / "a"));
    EXPECT_FALSE(TestFilesHelper::fileExists(dstPath / "a"));
}

TEST_F(ProcessorTest, givenFileWithExtensionWhenCopyingFileThenPreserveExtension) {
    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {createCopyAction("niceFile")};
    Processor processor{config, eventQueue, filesystem, &nullLogger};

    pushFileCreationEventAndCreateFile(srcPath / "a.jpg");
    pushInterruptEvent();
    processor.run();

    EXPECT_TRUE(TestFilesHelper::fileExists(srcPath / "a.jpg"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "niceFile.jpg"));
}

TEST_F(ProcessorTest, givenFileWithMultipleExtensionsWhenCopyingFileThenPreserveOnlyLastExtension) {
    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {createCopyAction("niceFile")};
    Processor processor{config, eventQueue, filesystem, &nullLogger};

    pushFileCreationEventAndCreateFile(srcPath / "a.gif.pdf.jpg");
    pushInterruptEvent();
    processor.run();

    EXPECT_TRUE(TestFilesHelper::fileExists(srcPath / "a.gif.pdf.jpg"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "niceFile.jpg"));
}

TEST_F(ProcessorTest, givenFileCopyActionTriggeredAndNameVariableUsedWhenProcessorIsRunningThenCopyFile) {
    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {createCopyAction("a_${name}_c")};
    Processor processor{config, eventQueue, filesystem, &nullLogger};

    pushFileCreationEventAndCreateFile(srcPath / "b");
    pushInterruptEvent();
    processor.run();

    EXPECT_TRUE(TestFilesHelper::fileExists(srcPath / "b"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "a_b_c"));
}

TEST_F(ProcessorTest, givenFileCopyActionTriggeredAndExtensionVariableUsedWhenProcessorIsRunningThenCopyFile) {
    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {createCopyAction("a_${extension}_c")};
    Processor processor{config, eventQueue, filesystem, &nullLogger};

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
    Processor processor{config, eventQueue, filesystem, &nullLogger};

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
    Processor processor{config, eventQueue, filesystem, &nullLogger};

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
    Processor processor{config, eventQueue, filesystem, &nullLogger};

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
    Processor processor{config, eventQueue, filesystem, &nullLogger};

    pushFileCreationEventAndCreateFile(srcPath / "a.pdf");
    pushFileCreationEventAndCreateFile(srcPath / "b.gif");
    pushInterruptEvent();
    processor.run();

    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "file_000.pdf"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "file_000.gif"));
    EXPECT_FALSE(TestFilesHelper::fileExists(dstPath / "file_001.pdf"));
    EXPECT_FALSE(TestFilesHelper::fileExists(dstPath / "file_001.gif"));
}

TEST_F(ProcessorTest, givenMultipleFileCopyActionsTriggeredAndCounterIsUsedWhenProcessorIsRunningThenCopyFile) {
    ProcessorConfig config = createProcessorConfigWithOneMatcher();
    config.matchers[0].actions = {createCopyAction("file_###")};
    Processor processor{config, eventQueue, filesystem, &nullLogger};

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
    Processor processor{config, eventQueue, filesystem, &nullLogger};

    pushFileCreationEventAndCreateFile(srcPath / "a.png");
    pushInterruptEvent();
    processor.run();

    EXPECT_FALSE(TestFilesHelper::fileExists(srcPath / "a.png"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "abc_0.png"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "abc_1.png"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "def.png"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "ghi_png.png"));
}
