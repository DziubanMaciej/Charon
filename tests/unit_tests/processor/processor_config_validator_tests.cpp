#include "charon/processor/processor_config_validator.h"
#include "unit_tests/fixtures/processor_config_fixture.h"
#include "unit_tests/mocks/mock_logger.h"

struct ProcessorConfigValidatorTest : ::testing::Test, ProcessorConfigFixture {
    void SetUp() override {
        ProcessorConfigFixture::SetUp();
    }

    const fs::path dummyDirWithNonAsciiCharacter = "dummy/path/\206abc";
};

TEST_F(ProcessorConfigValidatorTest, givenValidConfigWhenValidatingConfigThenReturnSuccess) {
    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();

    ProcessorConfig config = createProcessorConfigWithOneMatcher(dummyPath1);
    config.matchers[0].watchedExtensions = {"png", "jpg", "mp4", ""};
    config.matchers[0].actions = {
        createCopyAction(dummyPath2, "dst"),
        createCopyAction(dummyPath3, "dst"),
        createMoveAction(dummyPath3, "dst"),
        createPrintAction(),
        createPrintAction(),
    };

    EXPECT_TRUE(ProcessorConfigValidator::validateConfig(config));
}

TEST_F(ProcessorConfigValidatorTest, givenFilesystemActionAfterMoveOrRemoveWhenValidatingConfigThenReturnError) {
    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();

    EXPECT_CALL(logger, log(LogLevel::Error, "Placing actions after a move/remove action is illegal.")).Times(2);

    ProcessorConfig config = createProcessorConfigWithOneMatcher(dummyPath1);
    config.matchers[0].watchedExtensions = {"png", "jpg", "mp4", ""};
    config.matchers[0].actions = {
        createMoveAction(dummyPath3, "dst"),
        createMoveAction(dummyPath3, "dst"),
    };
    EXPECT_FALSE(ProcessorConfigValidator::validateConfig(config));

    config.matchers[0].actions = {
        createRemoveAction(),
        createCopyAction(dummyPath3, "dst"),
    };
    EXPECT_FALSE(ProcessorConfigValidator::validateConfig(config));
}

TEST_F(ProcessorConfigValidatorTest, givenExtensionWithDotWhenValidatingConfigThenReturnError) {
    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();

    EXPECT_CALL(logger, log(LogLevel::Error, "One of watched extensions contains a dot.")).Times(3);

    ProcessorConfig config = createProcessorConfigWithOneMatcher(dummyPath1);
    config.matchers[0].watchedExtensions = {"png", ".jpg", "mp4", ""};
    config.matchers[0].actions = {
        createCopyAction(dummyPath2, "dst"),
        createCopyAction(dummyPath3, "dst"),
        createMoveAction(dummyPath3, "dst"),
        createPrintAction(),
        createPrintAction(),
    };
    EXPECT_FALSE(ProcessorConfigValidator::validateConfig(config));

    config.matchers[0].watchedExtensions = {"so.png", "jpg", "mp4", ""};
    EXPECT_FALSE(ProcessorConfigValidator::validateConfig(config));

    config.matchers[0].watchedExtensions = {"png", "jpg", "mp4", "."};
    EXPECT_FALSE(ProcessorConfigValidator::validateConfig(config));
}

TEST_F(ProcessorConfigValidatorTest, givenExtensionWithNonAsciiCharacterWhenValidatingConfigThenReturnError) {
    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();

    EXPECT_CALL(logger, log(LogLevel::Error, "One of watched extensions contains illegal characters."));

    ProcessorConfig config = createProcessorConfigWithOneMatcher(dummyPath1);
    config.matchers[0].watchedExtensions = {"png", "jpg\206", "mp4", ""};
    config.matchers[0].actions = {
        createCopyAction(dummyPath2, "dst"),
        createCopyAction(dummyPath3, "dst"),
        createMoveAction(dummyPath3, "dst"),
        createPrintAction(),
        createPrintAction(),
    };
    EXPECT_FALSE(ProcessorConfigValidator::validateConfig(config));
}

TEST_F(ProcessorConfigValidatorTest, givenDestinationNamePatternWithNonAsciiCharacterWhenValidatingConfigThenReturnError) {
    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();

    EXPECT_CALL(logger, log(LogLevel::Error, "Destination name contains illegal characters.")).Times(2);

    ProcessorConfig config = createProcessorConfigWithOneMatcher(dummyPath1);
    config.matchers[0].watchedExtensions = {"png", "jpg", "mp4", ""};
    config.matchers[0].actions = {createCopyAction(dummyPath2, "dst\206")};
    EXPECT_FALSE(ProcessorConfigValidator::validateConfig(config));

    config.matchers[0].actions = {createMoveAction(dummyPath2, "dst\206")};
    EXPECT_FALSE(ProcessorConfigValidator::validateConfig(config));
}

TEST_F(ProcessorConfigValidatorTest, givenDestinationDirWithNonAsciiCharacterWhenValidatingConfigThenReturnError) {
    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();

    EXPECT_CALL(logger, log(LogLevel::Error, "Destination directory contains illegal characters.")).Times(2);

    ProcessorConfig config = createProcessorConfigWithOneMatcher(dummyPath1);
    config.matchers[0].watchedExtensions = {"png", "jpg", "mp4", ""};
    config.matchers[0].actions = {createCopyAction(dummyDirWithNonAsciiCharacter, "dst")};
    EXPECT_FALSE(ProcessorConfigValidator::validateConfig(config));

    config.matchers[0].actions = {createMoveAction(dummyDirWithNonAsciiCharacter, "dst")};
    EXPECT_FALSE(ProcessorConfigValidator::validateConfig(config));
}

TEST_F(ProcessorConfigValidatorTest, givenEmptyDestinationDirWhenValidatingConfigThenReturnError) {
    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();

    EXPECT_CALL(logger, log(LogLevel::Error, "Destination directory is empty.")).Times(2);

    ProcessorConfig config = createProcessorConfigWithOneMatcher(dummyPath1);
    config.matchers[0].watchedExtensions = {"png", "jpg", "mp4", ""};
    config.matchers[0].actions = {createCopyAction(fs::path(), "dst")};
    EXPECT_FALSE(ProcessorConfigValidator::validateConfig(config));

    config.matchers[0].actions = {createMoveAction(fs::path(), "dst")};
    EXPECT_FALSE(ProcessorConfigValidator::validateConfig(config));
}

TEST_F(ProcessorConfigValidatorTest, givenEmptyDestinationNamePatternWhenValidatingConfigThenReturnError) {
    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();

    EXPECT_CALL(logger, log(LogLevel::Error, "Destination name is empty.")).Times(2);

    ProcessorConfig config = createProcessorConfigWithOneMatcher(dummyPath1);
    config.matchers[0].watchedExtensions = {"png", "jpg", "mp4", ""};
    config.matchers[0].actions = {createCopyAction(dummyPath1, "")};
    EXPECT_FALSE(ProcessorConfigValidator::validateConfig(config));

    config.matchers[0].actions = {createMoveAction(dummyPath1, "")};
    EXPECT_FALSE(ProcessorConfigValidator::validateConfig(config));
}

TEST_F(ProcessorConfigValidatorTest, givenValidVariableInNamePatternWhenValidatingConfigThenReturnSuccess) {
    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();

    ProcessorConfig config = createProcessorConfigWithOneMatcher(dummyPath1);
    config.matchers[0].watchedExtensions = {"png", "jpg", "mp4", ""};
    config.matchers[0].actions = {
        createCopyAction(dummyPath2, "dst ${name}"),
        createCopyAction(dummyPath2, "dst ${previousName}"),
        createCopyAction(dummyPath2, "dst ${extension}"),
    };
    EXPECT_TRUE(ProcessorConfigValidator::validateConfig(config));
}

TEST_F(ProcessorConfigValidatorTest, givenInvalidVariableInNamePatternWhenValidatingConfigThenReturnError) {
    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();

    EXPECT_CALL(logger, log(LogLevel::Error, "Destination name contains illegal pseudo-variables.")).Times(2);

    ProcessorConfig config = createProcessorConfigWithOneMatcher(dummyPath1);
    config.matchers[0].watchedExtensions = {"png", "jpg", "mp4", ""};
    config.matchers[0].actions = {createCopyAction(dummyPath2, "dst${weirdVariable}")};
    EXPECT_FALSE(ProcessorConfigValidator::validateConfig(config));

    config.matchers[0].actions = {createMoveAction(dummyPath2, "dst${one} ${two}")};
    EXPECT_FALSE(ProcessorConfigValidator::validateConfig(config));
}

TEST_F(ProcessorConfigValidatorTest, givenUnclosedVariableInNamePatternWhenValidatingConfigThenReturnError) {
    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();

    EXPECT_CALL(logger, log(LogLevel::Error, "Destination name contains unclosed pseudo-variables.")).Times(3);

    ProcessorConfig config = createProcessorConfigWithOneMatcher(dummyPath1);
    config.matchers[0].watchedExtensions = {"png", "jpg", "mp4", ""};
    config.matchers[0].actions = {createCopyAction(dummyPath2, "dst ${")}; // unclosed and end of string
    EXPECT_FALSE(ProcessorConfigValidator::validateConfig(config));

    config.matchers[0].actions = {createCopyAction(dummyPath2, "dst ${aaa")}; // unclosed and some characters
    EXPECT_FALSE(ProcessorConfigValidator::validateConfig(config));

    config.matchers[0].actions = {createMoveAction(dummyPath2, "dst ${ ${name}")}; // unclosed and valid name after
    EXPECT_FALSE(ProcessorConfigValidator::validateConfig(config));
}

TEST_F(ProcessorConfigValidatorTest, givenContiguousHashesInNamePatternWhenValidatingConfigThenReturnSuccess) {
    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();

    ProcessorConfig config = createProcessorConfigWithOneMatcher(dummyPath1);
    config.matchers[0].watchedExtensions = {"png", "jpg", "mp4", ""};
    config.matchers[0].actions = {
        createCopyAction(dummyPath2, "dst #"),
        createCopyAction(dummyPath2, "dst ###"),
        createCopyAction(dummyPath2, "###"),
        createCopyAction(dummyPath2, "#"),
        createCopyAction(dummyPath2, "a_###_b"),
    };
    EXPECT_TRUE(ProcessorConfigValidator::validateConfig(config));
}

TEST_F(ProcessorConfigValidatorTest, givenUncontiguousHashesInNamePatternWhenValidatingConfigThenReturnError) {
    MockLogger logger{};
    auto loggerSetup = logger.raiiSetup();

    EXPECT_CALL(logger, log(LogLevel::Error, "Desination name contains uncontiguous hashes.")).Times(3);

    ProcessorConfig config = createProcessorConfigWithOneMatcher(dummyPath1);
    config.matchers[0].watchedExtensions = {"png", "jpg", "mp4", ""};
    config.matchers[0].actions = {createCopyAction(dummyPath2, "#_#")};
    EXPECT_FALSE(ProcessorConfigValidator::validateConfig(config));

    config.matchers[0].actions = {createCopyAction(dummyPath2, "abc_##_ad#")};
    EXPECT_FALSE(ProcessorConfigValidator::validateConfig(config));

    config.matchers[0].actions = {createCopyAction(dummyPath2, "abc_##_ad#aa")};
    EXPECT_FALSE(ProcessorConfigValidator::validateConfig(config));
}
