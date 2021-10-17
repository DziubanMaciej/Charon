#include "charon/watcher/directory_watcher.h"
#include "charon/watcher/directory_watcher_factory.h"
#include "os_tests/test_files_helper.h"

#include <gtest/gtest.h>

using namespace std::chrono_literals;

struct DirectoryWatcherTest : ::testing::Test {
    void SetUp() override {
        watchedDir = TestFilesHelper::createDirectory("dir");
        watcher = DirectoryWatcherFactoryImpl{}.create(watchedDir, eventQueue, deferredEventQueue);
    }

    const static inline auto popTimeoutDuration = 5ms;
    std::filesystem::path watchedDir{};
    FileEventQueue eventQueue{};
    FileEventQueue deferredEventQueue{};
    std::unique_ptr<DirectoryWatcher> watcher{};
};

TEST_F(DirectoryWatcherTest, givenFileCreatedWhenWatcherIsStartedThenPushEventToDeferredQueue) {
    EXPECT_TRUE(watcher->start());

    TestFilesHelper::createFile(watchedDir / "file");

    FileEvent event{};
    EXPECT_TRUE(deferredEventQueue.blockingPop(event, popTimeoutDuration));
    EXPECT_EQ(watchedDir, event.watchedRootPath);
    EXPECT_EQ(watchedDir / "file", event.path);
    EXPECT_EQ(FileEvent::Type::Add, event.type);
}

TEST_F(DirectoryWatcherTest, givenFileRemovedWhenWatcherIsStartedThenPushEventToNormalQueue) {
    TestFilesHelper::createFile(watchedDir / "file");

    EXPECT_TRUE(watcher->start());

    TestFilesHelper::removeFile(watchedDir / "file");

    FileEvent event{};
    EXPECT_TRUE(eventQueue.blockingPop(event, popTimeoutDuration));
    EXPECT_EQ(watchedDir, event.watchedRootPath);
    EXPECT_EQ(watchedDir / "file", event.path);
    EXPECT_EQ(FileEvent::Type::Remove, event.type);
}

TEST_F(DirectoryWatcherTest, givenFileRenamedWhenWatcherIsStartedThenPushEvents) {
    TestFilesHelper::createFile(watchedDir / "file");

    EXPECT_TRUE(watcher->start());

    TestFilesHelper::moveFile(watchedDir / "file",
                              watchedDir / "file2");

    FileEvent event{};
    EXPECT_TRUE(eventQueue.blockingPop(event, popTimeoutDuration));
    EXPECT_EQ(watchedDir, event.watchedRootPath);
    EXPECT_EQ(watchedDir / "file", event.path);
    EXPECT_EQ(FileEvent::Type::RenameOld, event.type);

    EXPECT_TRUE(deferredEventQueue.blockingPop(event, popTimeoutDuration));
    EXPECT_EQ(watchedDir, event.watchedRootPath);
    EXPECT_EQ(watchedDir / "file2", event.path);
    EXPECT_EQ(FileEvent::Type::RenameNew, event.type);
}

TEST_F(DirectoryWatcherTest, givenFileCreatedWhenWatcherIsNotStartedThenDoNotDetectEvent) {
    TestFilesHelper::createFile(watchedDir / "file");

    FileEvent event{};
    EXPECT_FALSE(eventQueue.blockingPop(event, popTimeoutDuration));
}

TEST_F(DirectoryWatcherTest, givenFileCreatedWhenWatcherIsStoppedThenDoNotDetectEvent) {
    EXPECT_TRUE(watcher->start());

    EXPECT_TRUE(watcher->stop());

    TestFilesHelper::createFile(watchedDir / "file");

    FileEvent event{};
    EXPECT_FALSE(eventQueue.blockingPop(event, popTimeoutDuration));
}

TEST_F(DirectoryWatcherTest, givenFileCreatedWhenWatcherIsRestartedThenDetectEvent) {
    EXPECT_TRUE(watcher->start());

    EXPECT_TRUE(watcher->stop());

    EXPECT_TRUE(watcher->start());

    TestFilesHelper::createFile(watchedDir / "file");

    FileEvent event{};
    EXPECT_TRUE(deferredEventQueue.blockingPop(event, popTimeoutDuration));
    EXPECT_EQ(watchedDir, event.watchedRootPath);
    EXPECT_EQ(watchedDir / "file", event.path);
    EXPECT_EQ(FileEvent::Type::Add, event.type);
}

TEST_F(DirectoryWatcherTest, givenMultipleFilesCreatedWhenWatcherIsActiveThenDetectAllEvents) {
    constexpr auto filesCount = 100u;

    EXPECT_TRUE(watcher->start());

    for (int i = 0; i < filesCount; i++) {
        TestFilesHelper::createFile(watchedDir / std::to_string(i));
    }

    for (int i = 0; i < filesCount; i++) {
        FileEvent event{};
        EXPECT_TRUE(deferredEventQueue.blockingPop(event, popTimeoutDuration));
        EXPECT_EQ(watchedDir, event.watchedRootPath);
        EXPECT_EQ(watchedDir / std::to_string(i), event.path);
        EXPECT_EQ(FileEvent::Type::Add, event.type);
    }
}
