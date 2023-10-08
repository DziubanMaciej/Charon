#include "charon/charon/charon.h"
#include "charon/util/filesystem_impl.h"
#include "charon/util/logger.h"
#include "charon/watcher/directory_watcher_factory.h"
#include "os_tests/fixtures/processor_config_fixture.h"

#include <gtest/gtest.h>

struct RaiiCharonRunner {
    RaiiCharonRunner(Charon &charon) : charon(charon) {
        EXPECT_TRUE(charon.start());
    }

    ~RaiiCharonRunner() {
        EXPECT_TRUE(charon.stop());
    }

    Charon &charon;
};

struct WhiteboxFilesystem : FilesystemImpl {
    OptionalError copy(const fs::path &src, const fs::path &dst) const override {
        copyCount++;
        return FilesystemImpl::copy(src, dst);
    }
    OptionalError move(const fs::path &src, const fs::path &dst) const override {
        moveCount++;
        return FilesystemImpl::move(src, dst);
    }
    OptionalError remove(const fs::path &file) const override {
        removeCount++;
        return FilesystemImpl::remove(file);
    }

    mutable size_t copyCount = 0u;
    mutable size_t moveCount = 0u;
    mutable size_t removeCount = 0u;
};

struct CharonOsTests : ::testing::Test,
                       ProcessorConfigFixture {
    void SetUp() override {
        ProcessorConfigFixture::SetUp();
    }

    void rerunCharon(Charon &charon) {
        // When Charon is running it may generate additional event (creating a feedback loop). We run it once again,
        // so it gets a chance to process them and handle accordingly.
        RaiiCharonRunner charonRunner{charon};
    }

    WhiteboxFilesystem filesystem;
    DirectoryWatcherFactoryImpl watcherFactory;
};

TEST_F(CharonOsTests, givenFileEventWhenCharonIsRunningThenExecuteActions) {
    ProcessorConfig processorConfig = createProcessorConfigWithOneMatcher();
    processorConfig.matchers()->matchers[0].actions = {
        createCopyAction("a"),
        createCopyAction("b"),
        createMoveAction("c"),
    };
    Charon charon{processorConfig, filesystem, watcherFactory};

    {
        RaiiCharonRunner charonRunner{charon};
        TestFilesHelper::createFile(srcPath / "abc");
    }
    rerunCharon(charon);

    EXPECT_FALSE(TestFilesHelper::fileExists(srcPath / "abc"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "a"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "b"));
    EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / "c"));
    EXPECT_EQ(2u, filesystem.copyCount);
    EXPECT_EQ(1u, filesystem.moveCount);
    EXPECT_EQ(0u, filesystem.removeCount);
}

TEST_F(CharonOsTests, givenRemoveActionWhenCharonIsRunningThenExecuteActions) {
    ProcessorConfig processorConfig = createProcessorConfigWithOneMatcher();
    processorConfig.matchers()->matchers[0].actions = {createRemoveAction()};
    Charon charon{processorConfig, filesystem, watcherFactory};

    {
        RaiiCharonRunner charonRunner{charon};
        TestFilesHelper::createFile(srcPath / "abc");
    }
    rerunCharon(charon);

    EXPECT_FALSE(TestFilesHelper::fileExists(srcPath / "abc"));
    EXPECT_EQ(0u, filesystem.copyCount);
    EXPECT_EQ(0u, filesystem.moveCount);
    EXPECT_EQ(1u, filesystem.removeCount);
}

TEST_F(CharonOsTests, givenMultipleFileEventsWhenCharonIsRunningThenExecuteActions) {
    ProcessorConfig processorConfig = createProcessorConfigWithOneMatcher();
    processorConfig.matchers()->matchers[0].actions = {createCopyAction("a#")};
    Charon charon{processorConfig, filesystem, watcherFactory};

    {
        RaiiCharonRunner charonRunner{charon};
        for (int i = 0; i < 7; i++) {
            TestFilesHelper::createFile(srcPath / (std::string("file") + std::to_string(i)));
        }
    }
    rerunCharon(charon);

    for (int i = 0; i < 7; i++) {
        EXPECT_TRUE(TestFilesHelper::fileExists(srcPath / (std::string("file") + std::to_string(i))));
        EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / (std::string("a") + std::to_string(i))));
    }
    EXPECT_EQ(7u, filesystem.copyCount);
    EXPECT_EQ(0u, filesystem.moveCount);
    EXPECT_EQ(0u, filesystem.removeCount);
}

TEST_F(CharonOsTests, givenMultipleFileEventsAndMultipleActionsWhenCharonIsRunningThenExecuteActions) {
    ProcessorConfig processorConfig = createProcessorConfigWithOneMatcher();
    processorConfig.matchers()->matchers[0].actions = {
        createCopyAction("a#"),
        createMoveAction("a${previousName}"),
    };
    Charon charon{processorConfig, filesystem, watcherFactory};

    {
        RaiiCharonRunner charonRunner{charon};
        for (int i = 0; i < 7; i++) {
            TestFilesHelper::createFile(srcPath / (std::string("file") + std::to_string(i)));
        }
    }
    rerunCharon(charon);

    for (int i = 0; i < 7; i++) {
        EXPECT_FALSE(TestFilesHelper::fileExists(srcPath / (std::string("file") + std::to_string(i))));
        EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / (std::string("a") + std::to_string(i))));
        EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / (std::string("aa") + std::to_string(i))));
    }
    EXPECT_EQ(7u, filesystem.copyCount);
    EXPECT_EQ(7u, filesystem.moveCount);
    EXPECT_EQ(0u, filesystem.removeCount);
}

TEST_F(CharonOsTests, givenFilesAreOpenedForSomeTimeWhenCharonIsMovingFilesThenResultsAreCorrect) {
    constexpr auto filesCount = 100u;

    ProcessorConfig processorConfig = createProcessorConfigWithOneMatcher();
    processorConfig.matchers()->matchers[0].actions = {createMoveAction("${name}")};
    Charon charon{processorConfig, filesystem, watcherFactory};

    {
        RaiiCharonRunner charonRunner{charon};

        std::vector<std::ofstream> files{};
        for (auto i = 0u; i < filesCount; i++) {
            files.push_back(TestFilesHelper::openFileForWriting(srcPath / std::to_string(i)));
        }
        for (auto i = 0u; i < filesCount; i++) {
            files[i] << i;
        }
        for (auto &file : files) {
            file.close();
        }
    }
    rerunCharon(charon);

    EXPECT_EQ(0u, TestFilesHelper::countFilesInDirectory(srcPath));
    EXPECT_EQ(filesCount, TestFilesHelper::countFilesInDirectory(dstPath));
    for (auto i = 0u; i < filesCount; i++) {
        EXPECT_TRUE(TestFilesHelper::fileContains(dstPath / std::to_string(i), std::to_string(i)));
    }
    EXPECT_EQ(0u, filesystem.copyCount);
    EXPECT_EQ(filesCount, filesystem.moveCount);
    EXPECT_EQ(0u, filesystem.removeCount);
}

TEST_F(CharonOsTests, givenFilesAreOpenedForSomeTimeWhenCharonIsCopyingFilesThenResultsAreCorrect) {
    constexpr auto filesCount = 100u;

    ProcessorConfig processorConfig = createProcessorConfigWithOneMatcher();
    processorConfig.matchers()->matchers[0].actions = {createCopyAction("${name}")};
    Charon charon{processorConfig, filesystem, watcherFactory};

    {
        RaiiCharonRunner charonRunner{charon};

        std::vector<std::ofstream> files{};
        for (auto i = 0u; i < filesCount; i++) {
            files.push_back(TestFilesHelper::openFileForWriting(srcPath / std::to_string(i)));
        }
        for (auto i = 0u; i < filesCount; i++) {
            files[i] << i;
        }
        for (auto &file : files) {
            file.close();
        }
    }
    rerunCharon(charon);

    EXPECT_EQ(filesCount, TestFilesHelper::countFilesInDirectory(srcPath));
    EXPECT_EQ(filesCount, TestFilesHelper::countFilesInDirectory(dstPath));
    for (auto i = 0u; i < filesCount; i++) {
        EXPECT_TRUE(TestFilesHelper::fileContains(srcPath / std::to_string(i), std::to_string(i)));
        EXPECT_TRUE(TestFilesHelper::fileContains(dstPath / std::to_string(i), std::to_string(i)));
    }
    EXPECT_EQ(filesCount, filesystem.copyCount);
    EXPECT_EQ(0u, filesystem.moveCount);
    EXPECT_EQ(0u, filesystem.removeCount);
}
