#include "charon/processor/processor_config_reader.h"
#include "unit_tests/mocks/mock_logger.h"

#include <gtest/gtest.h>

#define EXPECT_EMPTY(container) EXPECT_EQ(0u, container.size())

constexpr static inline ProcessorConfig::Type allProcessorConfigTypes[] = {
    ProcessorConfig::Type::Actions,
    ProcessorConfig::Type::Matchers,
};
constexpr static inline size_t processorConfigTypesCount = sizeof(allProcessorConfigTypes) / sizeof(allProcessorConfigTypes[0]);

TEST(ProcessorConfigReaderBasicTest, givenEmptyJsonWhenReadingConfigThenReturnError) {
    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();
    EXPECT_CALL(logger, log(LogLevel::Error, "Specified json was badly formed.")).Times(processorConfigTypesCount);

    for (auto type : allProcessorConfigTypes) {
        ProcessConfigReader reader{};
        ProcessorConfig config{};
        std::string json = "";
        EXPECT_FALSE(reader.read(config, json, type));
    }
}

TEST(ProcessorConfigReaderBasicTest, givenBadlyFormedJsonWhenReadingConfigThenReturnError) {
    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();
    EXPECT_CALL(logger, log(LogLevel::Error, "Specified json was badly formed.")).Times(processorConfigTypesCount);

    for (auto type : allProcessorConfigTypes) {
        ProcessConfigReader reader{};
        ProcessorConfig config{};
        std::string json = "[ { ]";
        EXPECT_FALSE(reader.read(config, json, type));
    }
}

TEST(ProcessorConfigReaderBadTypeTest, givenRootNodeIsNotAnArrayWhenReadingConfigThenReturnError) {
    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();
    EXPECT_CALL(logger, log(LogLevel::Error, "Root node must be an array.")).Times(3);
    EXPECT_CALL(logger, log(LogLevel::Error, "Actions list must be an array.")).Times(3);

    for (auto type : allProcessorConfigTypes) {
        ProcessConfigReader reader{};
        ProcessorConfig config{};

        std::string json = "{}";
        EXPECT_FALSE(reader.read(config, json, type));

        json = "1";
        EXPECT_FALSE(reader.read(config, json, type));

        json = "\"str\"";
        EXPECT_FALSE(reader.read(config, json, type));
    }
}

TEST(ProcessorConfigReaderBasicTest, givenEmptyArrayWhenReadingConfigWithMatchersThenReturnEmptyConfig) {
    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();
    EXPECT_CALL(logger, log).Times(0);

    ProcessConfigReader reader{};
    ProcessorConfig config{};
    std::string json = "[]";
    ASSERT_TRUE(reader.read(config, json, ProcessorConfig::Type::Matchers));
    EXPECT_EMPTY(config.matchers()->matchers);
}

TEST(ProcessorConfigReaderBadTypeTest, givenActionMatcherIsNotAnObjectWhenReadingConfigWithMatchersThenReturnError) {
    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();
    EXPECT_CALL(logger, log(LogLevel::Error, "Action matcher node must be an object"));

    ProcessConfigReader reader{};
    ProcessorConfig config{};
    std::string json = R"(
        [
            "foo"
        ]
    )";
    EXPECT_FALSE(reader.read(config, json, ProcessorConfig::Type::Matchers));
}

TEST(ProcessorConfigReaderBadTypeTest, givenExtensionsMemberIsNotAnArrayWhenReadingConfigWithMatchersThenReturnError) {
    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();
    EXPECT_CALL(logger, log(LogLevel::Error, "Action matcher \"extensions\" member must be an array."));

    ProcessConfigReader reader{};
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
    EXPECT_FALSE(reader.read(config, json, ProcessorConfig::Type::Matchers));
}

TEST(ProcessorConfigReaderBadTypeTest, givenActionsMemberIsNotAnArrayWhenReadingConfigWithMatchersThenReturnError) {
    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();
    EXPECT_CALL(logger, log(LogLevel::Error, "Actions list must be an array."));

    ProcessConfigReader reader{};
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
    EXPECT_FALSE(reader.read(config, json, ProcessorConfig::Type::Matchers));
}

TEST(ProcessorConfigReaderMissingFieldTest, givenNoWatchedFoldersFieldWhenReadingConfigWithMatchersThenReturnError) {
    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();
    EXPECT_CALL(logger, log(LogLevel::Error, "Action matcher node must contain \"watchedFolder\" field."));

    ProcessConfigReader reader{};
    ProcessorConfig config{};
    std::string json = R"(
        [
            {
                "extensions": [ "png", "jpg", "gif" ],
                "actions": []
            }
        ]
    )";
    EXPECT_FALSE(reader.read(config, json, ProcessorConfig::Type::Matchers));
}

TEST(ProcessorConfigReaderMissingFieldTest, givenNoExtensionsFieldWhenReadingConfigWithMatchersThenReturnSuccessAndEmptyExtensionsFilter) {
    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();
    EXPECT_CALL(logger, log).Times(0);

    ProcessConfigReader reader{};
    ProcessorConfig config{};
    std::string json = R"(
        [
            {
                "watchedFolder": "D:/Desktop/Test",
                "actions": []
            }
        ]
    )";
    ASSERT_TRUE(reader.read(config, json, ProcessorConfig::Type::Matchers));
    EXPECT_EQ(1u, config.matchers()->matchers.size());
    EXPECT_EMPTY(config.matchers()->matchers[0].watchedExtensions);
}

TEST(ProcessorConfigReaderMissingFieldTest, givenNoActionsFieldWhenReadingConfigWithMatchersThenReturnError) {
    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();
    EXPECT_CALL(logger, log(LogLevel::Error, "Action matcher node must contain \"actions\" field."));

    ProcessConfigReader reader{};
    ProcessorConfig config{};
    std::string json = R"(
        [
            {
                "watchedFolder": "D:/Desktop/Test",
                "extensions": [ "png", "jpg", "gif" ]
            }
        ]
    )";
    EXPECT_FALSE(reader.read(config, json, ProcessorConfig::Type::Matchers));
}

TEST(ProcessConfigReaderPositiveTest, givenCopyActionWhenReadingConfigWithMatchersThenParseCorrectly) {
    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();
    EXPECT_CALL(logger, log).Times(0);

    ProcessConfigReader reader{};
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
    ASSERT_TRUE(reader.read(config, json, ProcessorConfig::Type::Matchers));
    EXPECT_EQ(ProcessorAction::Type::Copy, config.matchers()->matchers[0].actions[0].type);
    auto data = std::get<ProcessorAction::MoveOrCopy>(config.matchers()->matchers[0].actions[0].data);
    EXPECT_EQ("D:/Desktop/Dst1", data.destinationDir);
    EXPECT_EQ("#.${ext}", data.destinationName);
}

TEST(ProcessConfigReaderPositiveTest, givenMoveActionWhenReadingConfigWithMatchersThenParseCorrectly) {
    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();
    EXPECT_CALL(logger, log).Times(0);

    ProcessConfigReader reader{};
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
    ASSERT_TRUE(reader.read(config, json, ProcessorConfig::Type::Matchers));
    EXPECT_EQ(ProcessorAction::Type::Move, config.matchers()->matchers[0].actions[0].type);
    auto data = std::get<ProcessorAction::MoveOrCopy>(config.matchers()->matchers[0].actions[0].data);
    EXPECT_EQ("D:/Desktop/Dst1", data.destinationDir);
    EXPECT_EQ("#.${ext}", data.destinationName);
}

TEST(ProcessConfigReaderPositiveTest, givenRemoveActionWhenReadingConfigWithMatchersThenParseCorrectly) {
    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();
    EXPECT_CALL(logger, log).Times(0);

    ProcessConfigReader reader{};
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
    ASSERT_TRUE(reader.read(config, json, ProcessorConfig::Type::Matchers));
    EXPECT_EQ(ProcessorAction::Type::Remove, config.matchers()->matchers[0].actions[0].type);
    EXPECT_TRUE(std::holds_alternative<ProcessorAction::Remove>(config.matchers()->matchers[0].actions[0].data));
}

TEST(ProcessConfigReaderPositiveTest, givenPrintActionWhenReadingConfigWithMatchersThenParseCorrectly) {
    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();
    EXPECT_CALL(logger, log).Times(0);

    ProcessConfigReader reader{};
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
    ASSERT_TRUE(reader.read(config, json, ProcessorConfig::Type::Matchers));
    EXPECT_EQ(ProcessorAction::Type::Print, config.matchers()->matchers[0].actions[0].type);
    EXPECT_TRUE(std::holds_alternative<ProcessorAction::Print>(config.matchers()->matchers[0].actions[0].data));
}

TEST(ProcessConfigReaderPositiveTest, givenComplexConfigWhenReadingConfigWithMatchersThenParseCorrectly) {
    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();
    EXPECT_CALL(logger, log).Times(0);

    ProcessConfigReader reader{};
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

    ASSERT_TRUE(reader.read(config, json, ProcessorConfig::Type::Matchers));
    EXPECT_EQ(2u, config.matchers()->matchers.size());

    {
        const auto &matcher = config.matchers()->matchers[0];
        EXPECT_EQ("D:/Desktop/Test1", matcher.watchedFolder);
        EXPECT_EQ((std::vector<fs::path>{"png", "jpg", "gif"}), matcher.watchedExtensions);
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
        const auto &matcher = config.matchers()->matchers[1];
        EXPECT_EQ("D:/Desktop/Test2", matcher.watchedFolder);
        EXPECT_EQ((std::vector<fs::path>{"mp4"}), matcher.watchedExtensions);
        EXPECT_EQ(1u, matcher.actions.size());
        {
            const auto &action = matcher.actions[0];
            EXPECT_TRUE(std::holds_alternative<ProcessorAction::Remove>(action.data));
            EXPECT_EQ(ProcessorAction::Type::Remove, action.type);
        }
    }
}

TEST(ProcessorConfigReaderBasicTest, givenEmptyArrayWhenReadingConfigWithActionsThenReturnEmptyConfig) {
    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();
    EXPECT_CALL(logger, log).Times(0);

    ProcessConfigReader reader{};
    ProcessorConfig config{};
    std::string json = "[]";
    ASSERT_TRUE(reader.read(config, json, ProcessorConfig::Type::Actions));
    EXPECT_EMPTY(config.actions()->actions);
}

TEST(ProcessConfigReaderPositiveTest, givenCopyActionWhenReadingConfigWithActionsThenParseCorrectly) {
    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();
    EXPECT_CALL(logger, log).Times(0);

    ProcessConfigReader reader{};
    ProcessorConfig config{};
    std::string json = R"(
        [
            {
                "type": "copy",
                "destinationDir": "D:/Desktop/Dst1",
                "destinationName": "#.${ext}"
            }
        ]
    )";
    ASSERT_TRUE(reader.read(config, json, ProcessorConfig::Type::Actions));
    ProcessorAction &action = config.actions()->actions[0];
    EXPECT_EQ(ProcessorAction::Type::Copy, action.type);
    auto data = std::get<ProcessorAction::MoveOrCopy>(action.data);
    EXPECT_EQ("D:/Desktop/Dst1", data.destinationDir);
    EXPECT_EQ("#.${ext}", data.destinationName);
}

TEST(ProcessConfigReaderPositiveTest, givenMultipleActionsWhenReadingConfigWithActionsThenParseCorrectly) {
    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();
    EXPECT_CALL(logger, log).Times(0);

    ProcessConfigReader reader{};
    ProcessorConfig config{};
    std::string json = R"(
        [
            {
                "type": "copy",
                "destinationDir": "D:/Desktop/Dst1",
                "destinationName": "#.${ext}"
            },
            {
                "type": "move",
                "destinationDir": "D:/Desktop/Dst2",
                "destinationName": "##.${ext}"
            },
            {
                "type": "remove"
            },
            {
                "type": "print"
            }
        ]
    )";
    ASSERT_TRUE(reader.read(config, json, ProcessorConfig::Type::Actions));
    {
        ProcessorAction &action = config.actions()->actions[0];
        EXPECT_EQ(ProcessorAction::Type::Copy, action.type);
        auto data = std::get<ProcessorAction::MoveOrCopy>(action.data);
        EXPECT_EQ("D:/Desktop/Dst1", data.destinationDir);
        EXPECT_EQ("#.${ext}", data.destinationName);
    }
    {
        ProcessorAction &action = config.actions()->actions[1];
        EXPECT_EQ(ProcessorAction::Type::Move, action.type);
        auto data = std::get<ProcessorAction::MoveOrCopy>(action.data);
        EXPECT_EQ("D:/Desktop/Dst2", data.destinationDir);
        EXPECT_EQ("##.${ext}", data.destinationName);
    }
    {
        ProcessorAction &action = config.actions()->actions[2];
        EXPECT_EQ(ProcessorAction::Type::Remove, action.type);
        EXPECT_TRUE(std::holds_alternative<ProcessorAction::Remove>(action.data));
    }
    {
        ProcessorAction &action = config.actions()->actions[3];
        EXPECT_EQ(ProcessorAction::Type::Print, action.type);
        EXPECT_TRUE(std::holds_alternative<ProcessorAction::Print>(action.data));
    }
}
