#pragma once

#include "charon/util/logger.h"

#include <gmock/gmock.h>

using ::testing::AnyNumber;
using ::testing::Exactly;

struct MockLogger : Logger {
    MockLogger(bool expectNoCalls = true) {
        const auto matcher = expectNoCalls ? Exactly(0) : AnyNumber();
        EXPECT_CALL(*this, log).Times(matcher);
    }

    MOCK_METHOD(void, log, (LogLevel logLevel, const std::string &message), (override));
};
