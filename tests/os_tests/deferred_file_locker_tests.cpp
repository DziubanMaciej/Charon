#include "charon/processor/deferred_file_locker.h"
#include "charon/util/filesystem_impl.h"
#include "charon/util/logger.h"
#include "os_tests/test_files_helper.h"
#include "os_tests/test_helpers.h"

#include <gtest/gtest.h>
#include <thread>

using namespace std::chrono_literals;

struct DeferredFileLockerTest : ::testing::Test {
    void SetUp() override {
        REQUIRE_FILE_LOCKING_OR_SKIP(filesystem);
    }

    void pushFileCreationEventAndCreateFile(const std::filesystem::path &path) {
        TestFilesHelper::createFile(path);
        inputQueue.push(FileEvent{testPath, FileEvent::Type::Add, path});
    }

    void pushInterruptEvent() {
        inputQueue.push(FileEvent::interruptEvent);
    }

    const static inline std::filesystem::path testPath = TEST_DIRECTORY_PATH;
    const static inline auto zeroTimeout = 0ms;
    const static inline auto hangTimeout = 5ms;
    FileEventQueue inputQueue{};
    FileEventQueue outputQueue{};
    FilesystemImpl filesystem{};
};

TEST_F(DeferredFileLockerTest, givenFileEventWhenDeferredFileLockerIsAlreadyRunningThenProcessEvent) {
    DeferredFileLocker fileLocker{inputQueue, outputQueue, filesystem};

    std::thread thread{[&]() {
        fileLocker.run();
    }};
    pushFileCreationEventAndCreateFile(testPath / "file");
    pushInterruptEvent();
    thread.join();

    FileEvent event{};
    EXPECT_TRUE(inputQueue.empty());
    EXPECT_TRUE(outputQueue.blockingPop(event, zeroTimeout));
    EXPECT_TRUE(outputQueue.empty());
    EXPECT_EQ(event.type, FileEvent::Type::Add);
    EXPECT_EQ(event.watchedRootPath, testPath);
    EXPECT_EQ(event.path, testPath / "file");
    EXPECT_NE(event.lockedFileHandle, defaultOsHandle);

    filesystem.unlockFile(event.lockedFileHandle);
}

TEST_F(DeferredFileLockerTest, givenFileEventWhenDeferredFileLockerIsNotYetRunningThenProcessEventWhenItStarts) {
    DeferredFileLocker fileLocker{inputQueue, outputQueue, filesystem};

    pushFileCreationEventAndCreateFile(testPath / "file");
    pushInterruptEvent();
    fileLocker.run();

    FileEvent event{};
    EXPECT_TRUE(inputQueue.empty());
    EXPECT_TRUE(outputQueue.blockingPop(event, zeroTimeout));
    EXPECT_TRUE(outputQueue.empty());
    EXPECT_EQ(event.type, FileEvent::Type::Add);
    EXPECT_EQ(event.watchedRootPath, testPath);
    EXPECT_EQ(event.path, testPath / "file");
    EXPECT_NE(event.lockedFileHandle, defaultOsHandle);

    filesystem.unlockFile(event.lockedFileHandle);
}

TEST_F(DeferredFileLockerTest, givenInterruptEventBeforeRegularEventsWhenDeferredFileLockerIsRunThenProcessRegularEvents) {
    DeferredFileLocker fileLocker{inputQueue, outputQueue, filesystem};

    pushFileCreationEventAndCreateFile(testPath / "0");
    pushFileCreationEventAndCreateFile(testPath / "1");
    pushFileCreationEventAndCreateFile(testPath / "2");
    pushInterruptEvent();
    pushFileCreationEventAndCreateFile(testPath / "3");
    pushFileCreationEventAndCreateFile(testPath / "4");
    pushFileCreationEventAndCreateFile(testPath / "5");
    fileLocker.run();

    EXPECT_TRUE(inputQueue.empty());
    for (auto i = 0u; i < 6; i++) {
        FileEvent event{};
        EXPECT_TRUE(outputQueue.blockingPop(event, zeroTimeout));
        EXPECT_EQ(event.type, FileEvent::Type::Add);
        EXPECT_EQ(event.watchedRootPath, testPath);
        EXPECT_EQ(event.path, testPath / std::to_string(i));
        EXPECT_NE(event.lockedFileHandle, defaultOsHandle);

        filesystem.unlockFile(event.lockedFileHandle);
    }
    EXPECT_TRUE(outputQueue.empty());
}

TEST_F(DeferredFileLockerTest, givenFileIsLockedWhenFileLockerProcessesEventThenDeferLockingUntilFileIsAvailable) {
    DeferredFileLocker fileLocker{inputQueue, outputQueue, filesystem};
    std::thread thread{[&]() {
        fileLocker.run();
    }};

    pushFileCreationEventAndCreateFile(testPath / "0");
    auto [lockedFileHandle, lockResult] = filesystem.lockFile(testPath / "0");
    ASSERT_EQ(lockResult, Filesystem::LockResult::Success);
    ASSERT_NE(lockedFileHandle, defaultOsHandle);
    pushFileCreationEventAndCreateFile(testPath / "1");

    // fileLocker should skip file "0", because it's locked and go to file "1"
    FileEvent event{};
    EXPECT_TRUE(outputQueue.blockingPop(event));
    EXPECT_EQ(event.path, testPath / "1");
    filesystem.unlockFile(event.lockedFileHandle);

    // File "0" is still locked, we shouldn't be able to get anything in the output queue
    EXPECT_FALSE(outputQueue.blockingPop(event, hangTimeout));

    // After unlocking, fileLocker can proceed. Let's push another event to wake the locker from sleep
    filesystem.unlockFile(lockedFileHandle);
    pushFileCreationEventAndCreateFile(testPath / "2");
    EXPECT_TRUE(outputQueue.blockingPop(event));
    EXPECT_EQ(event.path, testPath / "0");
    filesystem.unlockFile(event.lockedFileHandle);
    EXPECT_TRUE(outputQueue.blockingPop(event, zeroTimeout));
    EXPECT_EQ(event.path, testPath / "2");
    filesystem.unlockFile(event.lockedFileHandle);

    // Terminate fileLocker
    pushInterruptEvent();
    thread.join();
}
