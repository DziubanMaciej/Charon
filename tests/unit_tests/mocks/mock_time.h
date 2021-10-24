#pragma once

#include "charon/util/time.h"

#include <gmock/gmock.h>

using ::testing::AnyNumber;
using ::testing::Exactly;

struct MockTime : Time {
    MockTime(bool expectNoCalls = true) {
        const auto matcher = expectNoCalls ? Exactly(0) : AnyNumber();
        EXPECT_CALL(*this, writeCurrentTime).Times(matcher);
    }

    MOCK_METHOD(void, writeCurrentTime, (std::ostream & out), (const override));
};
