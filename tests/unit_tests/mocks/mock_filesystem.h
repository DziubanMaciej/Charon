#pragma once

#include "charon/util/filesystem.h"

#include <gmock/gmock.h>

using ::testing::AnyNumber;
using ::testing::Exactly;

struct MockFilesystem : Filesystem {
    MockFilesystem(bool expectNoCalls = true) {
        const auto matcher = expectNoCalls ? Exactly(0) : AnyNumber();
        EXPECT_CALL(*this, copy).Times(matcher);
        EXPECT_CALL(*this, move).Times(matcher);
        EXPECT_CALL(*this, remove).Times(matcher);
        EXPECT_CALL(*this, listFiles).Times(matcher);
        EXPECT_CALL(*this, lockFile).Times(matcher);
        EXPECT_CALL(*this, unlockFile).Times(matcher);

        EXPECT_CALL(*this, isDirectory).Times(AnyNumber());
    }

    MOCK_METHOD(OptionalError, copy, (const fs::path &src, const fs::path &dst), (const, override));
    MOCK_METHOD(OptionalError, move, (const fs::path &src, const fs::path &dst), (const, override));
    MOCK_METHOD(OptionalError, remove, (const fs::path &file), (const, override));
    MOCK_METHOD(bool, isDirectory, (const fs::path &path), (const, override));
    MOCK_METHOD(std::vector<fs::path>, listFiles, (const fs::path &directory), (const, override));

    MOCK_METHOD((std::pair<OsHandle, LockResult>), lockFile, (const fs::path &path), (const, override));
    MOCK_METHOD(void, unlockFile, (OsHandle & handle), (const, override));
};
