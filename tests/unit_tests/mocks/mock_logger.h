#pragma once

#include "charon/util/logger.h"

#include <gmock/gmock.h>

struct MockLogger : Logger {
    MockLogger() {
        EXPECT_CALL(*this, log).Times(0);
    }

    MOCK_METHOD(void, log, (const std::string &message), (override));
};
