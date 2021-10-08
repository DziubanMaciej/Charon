#include "charon/watcher/directory_watcher.h"
#include "os_tests/test_files_helper.h"

#include <gtest/gtest.h>

using namespace std::chrono_literals;

struct DirectoryWatcherTest : ::testing::Test {
    void SetUp() override {
        watchedDir = TestFilesHelper::createDirectory("dir");
        watcher = DirectoryWatcher::create(watchedDir, eventQueue);
    }

    void waitAfterWatcherStart() {
        std::this_thread::sleep_for(1ms);
    }

    const static inline auto popTimeoutDuration = 5ms;
    std::filesystem::path watchedDir{};
    FileEventQueue eventQueue{};
    std::unique_ptr<DirectoryWatcher> watcher{};
};

TEST_F(DirectoryWatcherTest, givenFileCreatedWhenWatcherIsStartedThenDetectEvent) {
    EXPECT_TRUE(watcher->start());
    waitAfterWatcherStart();

    TestFilesHelper::createFile(watchedDir / "file");

    FileEvent event{};
    EXPECT_TRUE(eventQueue.blockingPop(event, popTimeoutDuration));
    EXPECT_EQ(watchedDir, event.watchedRootPath);
    EXPECT_EQ(watchedDir / "file", event.path);
    EXPECT_EQ(FileEvent::Type::Add, event.type);
}

TEST_F(DirectoryWatcherTest, givenFileCreatedWhenWatcherIsNotStartedThenDoNotDetectEvent) {
    waitAfterWatcherStart();

    TestFilesHelper::createFile(watchedDir / "file");

    FileEvent event{};
    EXPECT_FALSE(eventQueue.blockingPop(event, popTimeoutDuration));
}

TEST_F(DirectoryWatcherTest, givenFileCreatedWhenWatcherIsStoppedThenDoNotDetectEvent) {
    EXPECT_TRUE(watcher->start());
    waitAfterWatcherStart();

    EXPECT_TRUE(watcher->stop());
    waitAfterWatcherStart();

    TestFilesHelper::createFile(watchedDir / "file");

    FileEvent event{};
    EXPECT_FALSE(eventQueue.blockingPop(event, popTimeoutDuration));
}

TEST_F(DirectoryWatcherTest, givenFileCreatedWhenWatcherIsRestartedThenDoNotDetectEvent) {
    EXPECT_TRUE(watcher->start());
    waitAfterWatcherStart();

    EXPECT_TRUE(watcher->stop());
    waitAfterWatcherStart();

    EXPECT_TRUE(watcher->start());
    waitAfterWatcherStart();

    TestFilesHelper::createFile(watchedDir / "file");

    FileEvent event{};
    EXPECT_TRUE(eventQueue.blockingPop(event, popTimeoutDuration));
    EXPECT_EQ(watchedDir, event.watchedRootPath);
    EXPECT_EQ(watchedDir / "file", event.path);
    EXPECT_EQ(FileEvent::Type::Add, event.type);
}

TEST_F(DirectoryWatcherTest, givenMultipleFilesCreatedWhenWatcherIsActiveThenDetectAllEvents) {
    EXPECT_TRUE(watcher->start());
    waitAfterWatcherStart();

    for (int i = 0; i < 256; i++) {
        TestFilesHelper::createFile(watchedDir / std::to_string(i));
    }

    for (int i = 0; i < 256; i++) {
        FileEvent event{};
        EXPECT_TRUE(eventQueue.blockingPop(event, popTimeoutDuration));
        EXPECT_EQ(watchedDir, event.watchedRootPath);
        EXPECT_EQ(watchedDir / std::to_string(i), event.path);
        EXPECT_EQ(FileEvent::Type::Add, event.type);
    }
}
