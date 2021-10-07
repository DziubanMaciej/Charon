#include "charon/processor/processor_config_reader.h"

#include <gtest/gtest.h>

#define EXPECT_EMPTY(container) EXPECT_EQ(0u, container.size())
#define EXPECT_ONE_PROCESSOR_ERROR(error) EXPECT_EQ(std::vector<std::string>{error}, reader.getErrors());

TEST(ProcessorConfigReaderBasicTest, givenEmptyJsonWhenReadingThenReturnError) {
    ProcessConfigReader reader{};
    ProcessorConfig config{};
    std::string json = "";
    EXPECT_FALSE(reader.read(config, json));
    EXPECT_ONE_PROCESSOR_ERROR("Specified json was badly formed.");
}

TEST(ProcessorConfigReaderBasicTest, givenBadlyFormedJsonWhenReadingThenReturnError) {
    ProcessConfigReader reader{};
    ProcessorConfig config{};
    std::string json = "[ { ]";
    EXPECT_FALSE(reader.read(config, json));
    EXPECT_ONE_PROCESSOR_ERROR("Specified json was badly formed.");
}

TEST(ProcessorConfigReaderBasicTest, givenEmptyArrayWhenReadingThenReturnEmptyConfig) {
    ProcessConfigReader reader{};
    ProcessorConfig config{};
    std::string json = "[]";
    ASSERT_TRUE(reader.read(config, json));
    EXPECT_EMPTY(reader.getErrors());
    EXPECT_EMPTY(config.entries);
}

TEST(ProcessorConfigReaderBadTypeTest, givenRootNodeIsNotAnArrayWhenReadingThenReturnError) {
    ProcessConfigReader reader{};
    ProcessorConfig config{};

    std::string json = "{}";
    EXPECT_FALSE(reader.read(config, json));
    EXPECT_ONE_PROCESSOR_ERROR("Root node must be an array");

    json = "1";
    EXPECT_FALSE(reader.read(config, json));
    EXPECT_ONE_PROCESSOR_ERROR("Root node must be an array");

    json = "\"str\"";
    EXPECT_FALSE(reader.read(config, json));
    EXPECT_ONE_PROCESSOR_ERROR("Root node must be an array");
}

TEST(ProcessorConfigReaderBadTypeTest, givenConfigEntryIsNotAnObjectWhenReadingThenReturnError) {
    ProcessConfigReader reader{};
    ProcessorConfig config{};
    std::string json = R"(
        [
            "foo"
        ]
    )";
    EXPECT_FALSE(reader.read(config, json));
    EXPECT_ONE_PROCESSOR_ERROR("Config entry node must be an object");
}

TEST(ProcessorConfigReaderBadTypeTest, givenExtensionsMemberIsNotAnArrayWhenReadingThenReturnError) {
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
    EXPECT_FALSE(reader.read(config, json));
    EXPECT_ONE_PROCESSOR_ERROR("Config entry \"extensions\" member must be an array.");
}

TEST(ProcessorConfigReaderBadTypeTest, givenActionsMemberIsNotAnArrayWhenReadingThenReturnError) {
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
    EXPECT_FALSE(reader.read(config, json));
    EXPECT_ONE_PROCESSOR_ERROR("Config entry \"actions\" member must be an array.");
}

TEST(ProcessorConfigReaderMissingFieldTest, givenNoWatchedFoldersFieldWhenReadingThenReturnError) {
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
    EXPECT_FALSE(reader.read(config, json));
    EXPECT_ONE_PROCESSOR_ERROR("Config entry node must contain \"watchedFolder\" field.");
}

TEST(ProcessorConfigReaderMissingFieldTest, givenNoExtensionsFieldWhenReadingThenReturnSuccessAndEmptyExtensionsFilter) {
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
    ASSERT_TRUE(reader.read(config, json));
    EXPECT_EMPTY(reader.getErrors());
    EXPECT_EQ(1u, config.entries.size());
    EXPECT_EMPTY(config.entries[0].watchedExtensions);
}

TEST(ProcessorConfigReaderMissingFieldTest, givenNoActionsFieldWhenReadingThenReturnError) {
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
    EXPECT_FALSE(reader.read(config, json));
    EXPECT_ONE_PROCESSOR_ERROR("Config entry node must contain \"actions\" field.");
}

TEST(ProcessorConfigReaderMissingFieldTest, givenNoOverrideExistingFieldWhenReadingThenReturnSuccessAndSetValueToFalse) {
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
    ASSERT_TRUE(reader.read(config, json));
    EXPECT_EMPTY(reader.getErrors());
    EXPECT_FALSE(std::get<ProcessorAction::MoveOrCopy>(config.entries[0].actions[0].data).overwriteExisting);
}

TEST(ProcessConfigReaderPositiveTest, givenComplexConfigWhenReadingThenParseCorrectly) {
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
                        "destinationName": "#.${ext}",
                        "overwriteExisting": false
                    },
                    {
                        "type": "move",
                        "destinationDir": "D:/Desktop/Dst2",
                        "destinationName": "##.${ext}",
                        "overwriteExisting": true
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
    EXPECT_EMPTY(reader.getErrors());
    EXPECT_EQ(2u, config.entries.size());

    {
        const auto &entry = config.entries[0];
        EXPECT_EQ("D:/Desktop/Test1", entry.watchedFolder);
        EXPECT_EQ((std::vector<std::string>{"png", "jpg", "gif"}), entry.watchedExtensions);
        EXPECT_EQ(2u, entry.actions.size());
        {
            const auto &action = entry.actions[0];
            const auto actionData = std::get<ProcessorAction::MoveOrCopy>(action.data);
            EXPECT_EQ(ProcessorAction::Type::Copy, action.type);
            EXPECT_EQ("D:/Desktop/Dst1", actionData.destinationDir);
            EXPECT_EQ("#.${ext}", actionData.destinationName);
            EXPECT_FALSE(actionData.overwriteExisting);
        }
        {
            const auto &action = entry.actions[1];
            const auto actionData = std::get<ProcessorAction::MoveOrCopy>(action.data);
            EXPECT_EQ(ProcessorAction::Type::Move, action.type);
            EXPECT_EQ("D:/Desktop/Dst2", actionData.destinationDir);
            EXPECT_EQ("##.${ext}", actionData.destinationName);
            EXPECT_TRUE(actionData.overwriteExisting);
        }
    }

    {
        const auto &entry = config.entries[1];
        EXPECT_EQ("D:/Desktop/Test2", entry.watchedFolder);
        EXPECT_EQ((std::vector<std::string>{"mp4"}), entry.watchedExtensions);
        EXPECT_EQ(1u, entry.actions.size());
        {
            const auto &action = entry.actions[0];
            EXPECT_TRUE(std::holds_alternative<ProcessorAction::Remove>(action.data));
            EXPECT_EQ(ProcessorAction::Type::Remove, action.type);
        }
    }
}
