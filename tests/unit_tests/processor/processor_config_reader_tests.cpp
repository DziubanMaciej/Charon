#include "charon/processor/processor_config_reader.h"
#include "unit_tests/mocks/mock_logger.h"

#include <gtest/gtest.h>

#define EXPECT_EMPTY(container) EXPECT_EQ(0u, container.size())

TEST(ProcessorConfigReaderBasicTest, givenEmptyJsonWhenReadingThenReturnError) {
    MockLogger logger{};
    EXPECT_CALL(logger, log(LogLevel::Error, "Specified json was badly formed."));

    ProcessConfigReader reader{logger};
    ProcessorConfig config{};
    std::string json = "";
    EXPECT_FALSE(reader.read(config, json));
}

TEST(ProcessorConfigReaderBasicTest, givenBadlyFormedJsonWhenReadingThenReturnError) {
    MockLogger logger{};
    EXPECT_CALL(logger, log(LogLevel::Error, "Specified json was badly formed."));

    ProcessConfigReader reader{logger};
    ProcessorConfig config{};
    std::string json = "[ { ]";
    EXPECT_FALSE(reader.read(config, json));
}

TEST(ProcessorConfigReaderBasicTest, givenEmptyArrayWhenReadingThenReturnEmptyConfig) {
    MockLogger logger{};
    EXPECT_CALL(logger, log).Times(0);

    ProcessConfigReader reader{logger};
    ProcessorConfig config{};
    std::string json = "[]";
    ASSERT_TRUE(reader.read(config, json));
    EXPECT_EMPTY(config.matchers);
}

TEST(ProcessorConfigReaderBadTypeTest, givenRootNodeIsNotAnArrayWhenReadingThenReturnError) {
    MockLogger logger{};
    EXPECT_CALL(logger, log(LogLevel::Error, "Root node must be an array")).Times(3);

    ProcessConfigReader reader{logger};
    ProcessorConfig config{};

    std::string json = "{}";
    EXPECT_FALSE(reader.read(config, json));

    json = "1";
    EXPECT_FALSE(reader.read(config, json));

    json = "\"str\"";
    EXPECT_FALSE(reader.read(config, json));
}

TEST(ProcessorConfigReaderBadTypeTest, givenActionMatcherIsNotAnObjectWhenReadingThenReturnError) {
    MockLogger logger{};
    EXPECT_CALL(logger, log(LogLevel::Error, "Action matcher node must be an object"));

    ProcessConfigReader reader{logger};
    ProcessorConfig config{};
    std::string json = R"(
        [
            "foo"
        ]
    )";
    EXPECT_FALSE(reader.read(config, json));
}

TEST(ProcessorConfigReaderBadTypeTest, givenExtensionsMemberIsNotAnArrayWhenReadingThenReturnError) {
    MockLogger logger{};
    EXPECT_CALL(logger, log(LogLevel::Error, "Action matcher \"extensions\" member must be an array."));

    ProcessConfigReader reader{logger};
    ProcessorConfig config{};
    std::string json = R"(
        [
            {
                "extensions": "jpg",
                "watchedFolder": "D:/Desktop/Test",
                "actions": []
            }
        ]
    )";
    EXPECT_FALSE(reader.read(config, json));
}

TEST(ProcessorConfigReaderBadTypeTest, givenActionsMemberIsNotAnArrayWhenReadingThenReturnError) {
    MockLogger logger{};
    EXPECT_CALL(logger, log(LogLevel::Error, "Action matcher \"actions\" member must be an array."));

    ProcessConfigReader reader{logger};
    ProcessorConfig config{};
    std::string json = R"(
        [
            {
                "extensions": [ "png", "jpg", "gif" ],
                "watchedFolder": "D:/Desktop/Test",
                "actions": {}
            }
        ]
    )";
    EXPECT_FALSE(reader.read(config, json));
}

TEST(ProcessorConfigReaderMissingFieldTest, givenNoWatchedFoldersFieldWhenReadingThenReturnError) {
    MockLogger logger{};
    EXPECT_CALL(logger, log(LogLevel::Error, "Action matcher node must contain \"watchedFolder\" field."));

    ProcessConfigReader reader{logger};
    ProcessorConfig config{};
    std::string json = R"(
        [
            {
                "extensions": [ "png", "jpg", "gif" ],
                "actions": []
            }
        ]
    )";
    EXPECT_FALSE(reader.read(config, json));
}

TEST(ProcessorConfigReaderMissingFieldTest, givenNoExtensionsFieldWhenReadingThenReturnSuccessAndEmptyExtensionsFilter) {
    MockLogger logger{};
    EXPECT_CALL(logger, log).Times(0);

    ProcessConfigReader reader{logger};
    ProcessorConfig config{};
    std::string json = R"(
        [
            {
                "watchedFolder": "D:/Desktop/Test",
                "actions": []
            }
        ]
    )";
    ASSERT_TRUE(reader.read(config, json));
    EXPECT_EQ(1u, config.matchers.size());
    EXPECT_EMPTY(config.matchers[0].watchedExtensions);
}

TEST(ProcessorConfigReaderMissingFieldTest, givenNoActionsFieldWhenReadingThenReturnError) {
    MockLogger logger{};
    EXPECT_CALL(logger, log(LogLevel::Error, "Action matcher node must contain \"actions\" field."));

    ProcessConfigReader reader{logger};
    ProcessorConfig config{};
    std::string json = R"(
        [
            {
                "watchedFolder": "D:/Desktop/Test",
                "extensions": [ "png", "jpg", "gif" ]
            }
        ]
    )";
    EXPECT_FALSE(reader.read(config, json));
}

TEST(ProcessConfigReaderPositiveTest, givenCopyActionWhenReadingThenParseCorrectly) {
    MockLogger logger{};
    EXPECT_CALL(logger, log).Times(0);

    ProcessConfigReader reader{logger};
    ProcessorConfig config{};
    std::string json = R"(
        [
            {
                "watchedFolder": "D:/Desktop/Test",
                "extensions": [ "png", "jpg", "gif" ],
                "actions": [
                    {
                        "type": "copy",
                        "destinationDir": "D:/Desktop/Dst1",
                        "destinationName": "#.${ext}"
                    }
                ]
            }
        ]
    )";
    ASSERT_TRUE(reader.read(config, json));
    EXPECT_EQ(ProcessorAction::Type::Copy, config.matchers[0].actions[0].type);
    auto data = std::get<ProcessorAction::MoveOrCopy>(config.matchers[0].actions[0].data);
    EXPECT_EQ("D:/Desktop/Dst1", data.destinationDir);
    EXPECT_EQ("#.${ext}", data.destinationName);
}

TEST(ProcessConfigReaderPositiveTest, givenMoveActionWhenReadingThenParseCorrectly) {
    MockLogger logger{};
    EXPECT_CALL(logger, log).Times(0);

    ProcessConfigReader reader{logger};
    ProcessorConfig config{};
    std::string json = R"(
        [
            {
                "watchedFolder": "D:/Desktop/Test",
                "extensions": [ "png", "jpg", "gif" ],
                "actions": [
                    {
                        "type": "move",
                        "destinationDir": "D:/Desktop/Dst1",
                        "destinationName": "#.${ext}"
                    }
                ]
            }
        ]
    )";
    ASSERT_TRUE(reader.read(config, json));
    EXPECT_EQ(ProcessorAction::Type::Move, config.matchers[0].actions[0].type);
    auto data = std::get<ProcessorAction::MoveOrCopy>(config.matchers[0].actions[0].data);
    EXPECT_EQ("D:/Desktop/Dst1", data.destinationDir);
    EXPECT_EQ("#.${ext}", data.destinationName);
}

TEST(ProcessConfigReaderPositiveTest, givenRemoveActionWhenReadingThenParseCorrectly) {
    MockLogger logger{};
    EXPECT_CALL(logger, log).Times(0);

    ProcessConfigReader reader{logger};
    ProcessorConfig config{};
    std::string json = R"(
        [
            {
                "watchedFolder": "D:/Desktop/Test",
                "extensions": [ "png", "jpg", "gif" ],
                "actions": [
                    {
                        "type": "remove"
                    }
                ]
            }
        ]
    )";
    ASSERT_TRUE(reader.read(config, json));
    EXPECT_EQ(ProcessorAction::Type::Remove, config.matchers[0].actions[0].type);
    EXPECT_TRUE(std::holds_alternative<ProcessorAction::Remove>(config.matchers[0].actions[0].data));
}

TEST(ProcessConfigReaderPositiveTest, givenPrintActionWhenReadingThenParseCorrectly) {
    MockLogger logger{};
    EXPECT_CALL(logger, log).Times(0);

    ProcessConfigReader reader{logger};
    ProcessorConfig config{};
    std::string json = R"(
        [
            {
                "watchedFolder": "D:/Desktop/Test",
                "extensions": [ "png", "jpg", "gif" ],
                "actions": [
                    {
                        "type": "print"
                    }
                ]
            }
        ]
    )";
    ASSERT_TRUE(reader.read(config, json));
    EXPECT_EQ(ProcessorAction::Type::Print, config.matchers[0].actions[0].type);
    EXPECT_TRUE(std::holds_alternative<ProcessorAction::Print>(config.matchers[0].actions[0].data));
}

TEST(ProcessConfigReaderPositiveTest, givenComplexConfigWhenReadingThenParseCorrectly) {
    MockLogger logger{};
    EXPECT_CALL(logger, log).Times(0);

    ProcessConfigReader reader{logger};
    ProcessorConfig config{};
    std::string json = R"(
        [
            {
                "watchedFolder": "D:/Desktop/Test1",
                "extensions": [ "png", "jpg", "gif" ],
                "actions": [
                    {
                        "type": "copy",
                        "destinationDir": "D:/Desktop/Dst1",
                        "destinationName": "#.${ext}"
                    },
                    {
                        "type": "move",
                        "destinationDir": "D:/Desktop/Dst2",
                        "destinationName": "##.${ext}"
                    }
                ]
            },
            {
                "watchedFolder": "D:/Desktop/Test2",
                "extensions": [ "mp4" ],
                "actions": [
                    {
                        "type": "remove"
                    }
                ]
            }
        ]
    )";

    ASSERT_TRUE(reader.read(config, json));
    EXPECT_EQ(2u, config.matchers.size());

    {
        const auto &matcher = config.matchers[0];
        EXPECT_EQ("D:/Desktop/Test1", matcher.watchedFolder);
        EXPECT_EQ((std::vector<std::string>{"png", "jpg", "gif"}), matcher.watchedExtensions);
        EXPECT_EQ(2u, matcher.actions.size());
        {
            const auto &action = matcher.actions[0];
            const auto actionData = std::get<ProcessorAction::MoveOrCopy>(action.data);
            EXPECT_EQ(ProcessorAction::Type::Copy, action.type);
            EXPECT_EQ("D:/Desktop/Dst1", actionData.destinationDir);
            EXPECT_EQ("#.${ext}", actionData.destinationName);
        }
        {
            const auto &action = matcher.actions[1];
            const auto actionData = std::get<ProcessorAction::MoveOrCopy>(action.data);
            EXPECT_EQ(ProcessorAction::Type::Move, action.type);
            EXPECT_EQ("D:/Desktop/Dst2", actionData.destinationDir);
            EXPECT_EQ("##.${ext}", actionData.destinationName);
        }
    }

    {
        const auto &matcher = config.matchers[1];
        EXPECT_EQ("D:/Desktop/Test2", matcher.watchedFolder);
        EXPECT_EQ((std::vector<std::string>{"mp4"}), matcher.watchedExtensions);
        EXPECT_EQ(1u, matcher.actions.size());
        {
            const auto &action = matcher.actions[0];
            EXPECT_TRUE(std::holds_alternative<ProcessorAction::Remove>(action.data));
            EXPECT_EQ(ProcessorAction::Type::Remove, action.type);
        }
    }
}
