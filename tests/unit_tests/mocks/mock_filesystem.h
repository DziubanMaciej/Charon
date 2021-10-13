#pragma once

#include "charon/util/filesystem.h"

#include <gmock/gmock.h>

using ::testing::AtLeast;
using ::testing::Return;

struct MockFilesystem : Filesystem {
    MockFilesystem(bool expectNoCalls = true) {
        if (expectNoCalls) {
            EXPECT_CALL(*this, copy).Times(0);
            EXPECT_CALL(*this, move).Times(0);
            EXPECT_CALL(*this, remove).Times(0);
            EXPECT_CALL(*this, listFiles).Times(0);
            EXPECT_CALL(*this, isDirectory).Times(AtLeast(0)).WillRepeatedly(Return(false));
        }
    }

    MOCK_METHOD(void, copy, (const fs::path &src, const fs::path &dst), (const, override));
    MOCK_METHOD(void, move, (const fs::path &src, const fs::path &dst), (const, override));
    MOCK_METHOD(void, remove, (const fs::path &file), (const, override));
    MOCK_METHOD(bool, isDirectory, (const fs::path &path), (const, override));
    MOCK_METHOD(std::vector<fs::path>, listFiles, (const fs::path &directory), (const, override));
};
