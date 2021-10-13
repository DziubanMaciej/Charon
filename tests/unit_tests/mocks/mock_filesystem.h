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

        EXPECT_CALL(*this, isDirectory).Times(AnyNumber());
    }

    MOCK_METHOD(void, copy, (const fs::path &src, const fs::path &dst), (const, override));
    MOCK_METHOD(void, move, (const fs::path &src, const fs::path &dst), (const, override));
    MOCK_METHOD(void, remove, (const fs::path &file), (const, override));
    MOCK_METHOD(bool, isDirectory, (const fs::path &path), (const, override));
    MOCK_METHOD(std::vector<fs::path>, listFiles, (const fs::path &directory), (const, override));
};
