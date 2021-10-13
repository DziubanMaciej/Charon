#include "charon/charon/charon.h"
#include "charon/util/filesystem_impl.h"
#include "charon/util/logger.h"
#include "charon/watcher/directory_watcher_factory.h"
#include "os_tests/fixtures/processor_config_fixture.h"

#include <gtest/gtest.h>

struct RaiiCharonRunner {
    RaiiCharonRunner(Charon &charon)
        : charon(charon),
          charonProcessorThread(runCharonInBackground(charon)) {
        waitAfterWatcherStart();
    }

    ~RaiiCharonRunner() {
        if (isRunning()) {
            charon.stopProcessor();
            charonProcessorThread->join();
        }
    }

    bool isRunning() const {
        return charonProcessorThread != nullptr;
    }

private:
    void waitAfterWatcherStart() {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1ms);
    }

    std::unique_ptr<std::thread> runCharonInBackground(Charon &charon) {
        if (!charon.runWatchers()) {
            return {};
        }
        return std::make_unique<std::thread>([&charon]() { charon.runProcessor(); });
    }

    Charon &charon;
    std::unique_ptr<std::thread> charonProcessorThread;
};

struct WhiteboxFilesystem : FilesystemImpl {
    void copy(const fs::path &src, const fs::path &dst) const override {
        copyCount++;
        FilesystemImpl::copy(src, dst);
    }
    void move(const fs::path &src, const fs::path &dst) const override {
        moveCount++;
        FilesystemImpl::move(src, dst);
    }
    void remove(const fs::path &file) const override {
        removeCount++;
        FilesystemImpl::remove(file);
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

    void waitForProcessor() {
        // We technically could give RaiiCharonRunner a narrower scope to execute its destructor earlier
        // and terminate Processor by inserting interrupt event and joining the thread. This however doesn't
        // let us test for eventual feedback loops (events created during handling events, e.g. during moving files).
        // To capture this, we would have to insert the interrupt event after the Processor handles all events
        // and DirectoryWatcher generates new. This is quite hard to check without adding some additional
        // convoluted logic to the Processor.
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(10ms);
    }

    WhiteboxFilesystem filesystem;
    NullLogger logger;
    DirectoryWatcherFactoryImpl watcherFactory;
};

TEST_F(CharonOsTests, givenFileEventWhenCharonIsRunningThenExecuteActions) {
    ProcessorConfig processorConfig = createProcessorConfigWithOneMatcher();
    processorConfig.matchers[0].actions = {
        createCopyAction("a"),
        createCopyAction("b"),
        createMoveAction("c"),
    };
    Charon charon{processorConfig, filesystem, logger, watcherFactory};
    RaiiCharonRunner charonRunner{charon};
    ASSERT_TRUE(charonRunner.isRunning());
    TestFilesHelper::createFile(srcPath / "abc");

    waitForProcessor();

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
    processorConfig.matchers[0].actions = {createRemoveAction()};
    Charon charon{processorConfig, filesystem, logger, watcherFactory};
    RaiiCharonRunner charonRunner{charon};
    ASSERT_TRUE(charonRunner.isRunning());
    TestFilesHelper::createFile(srcPath / "abc");

    waitForProcessor();

    EXPECT_FALSE(TestFilesHelper::fileExists(srcPath / "abc"));
    EXPECT_EQ(0u, filesystem.copyCount);
    EXPECT_EQ(0u, filesystem.moveCount);
    EXPECT_EQ(1u, filesystem.removeCount);
}

TEST_F(CharonOsTests, givenMultipleFileEventsWhenCharonIsRunningThenExecuteActions) {
    ProcessorConfig processorConfig = createProcessorConfigWithOneMatcher();
    processorConfig.matchers[0].actions = {createCopyAction("a#")};
    Charon charon{processorConfig, filesystem, logger, watcherFactory};
    RaiiCharonRunner charonRunner{charon};
    ASSERT_TRUE(charonRunner.isRunning());
    for (int i = 0; i < 7; i++) {
        TestFilesHelper::createFile(srcPath / (std::string("file") + std::to_string(i)));
    }

    waitForProcessor();

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
    processorConfig.matchers[0].actions = {
        createCopyAction("a#"),
        createMoveAction("a${previousName}"),
    };
    Charon charon{processorConfig, filesystem, logger, watcherFactory};
    RaiiCharonRunner charonRunner{charon};
    ASSERT_TRUE(charonRunner.isRunning());
    for (int i = 0; i < 7; i++) {
        TestFilesHelper::createFile(srcPath / (std::string("file") + std::to_string(i)));
    }

    waitForProcessor();

    for (int i = 0; i < 7; i++) {
        EXPECT_FALSE(TestFilesHelper::fileExists(srcPath / (std::string("file") + std::to_string(i))));
        EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / (std::string("a") + std::to_string(i))));
        EXPECT_TRUE(TestFilesHelper::fileExists(dstPath / (std::string("aa") + std::to_string(i))));
    }
    EXPECT_EQ(7u, filesystem.copyCount);
    EXPECT_EQ(7u, filesystem.moveCount);
    EXPECT_EQ(0u, filesystem.removeCount);
}
