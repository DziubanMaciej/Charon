#pragma once

#include "charon/processor/processor_config.h"
#include "charon/util/class_traits.h"

#include <nlohmann/json.hpp>

class ProcessConfigReader : NonCopyableAndMovable {
public:
    ProcessConfigReader() = default;

    bool read(ProcessorConfig &outConfig, const std::filesystem::path &jsonFile, ProcessorConfig::Type type);
    bool read(ProcessorConfig &outConfig, const std::string &json, ProcessorConfig::Type type);

private:
    bool parseProcessorConfigMatchers(ProcessorConfig &outConfig, const nlohmann::json &node);
    bool parseProcessorConfigActions(ProcessorConfig &outConfig, const nlohmann::json &node);

    bool parseProcessorActionMatcher(ProcessorActionMatcher &outActionMatcher, const nlohmann::json &node);
    bool parseProcessorActions(std::vector<ProcessorAction> &outActions, const nlohmann::json &node);
    bool parseProcessorAction(ProcessorAction &outAction, const nlohmann::json &node);

    static bool readFile(const std::filesystem::path &jsonFile, std::string &outContent);
};
