#pragma once

#include "charon/processor/processor_config.h"
#include "charon/util/class_traits.h"

#include <nlohmann/json.hpp>

struct Logger;

class ProcessConfigReader : NonCopyableAndMovable {
public:
    ProcessConfigReader(Logger &logger);

    bool read(ProcessorConfig &outConfig, const std::filesystem::path &jsonFile);
    bool read(ProcessorConfig &outConfig, const std::string &json);

private:
    bool parseProcessorConfig(ProcessorConfig &outConfig, const nlohmann::json &node);
    bool parseProcessorActionMatcher(ProcessorActionMatcher &outActionMatcher, const nlohmann::json &node);
    bool parseProcessorAction(ProcessorAction &outAction, const nlohmann::json &node);

    static bool readFile(const std::filesystem::path &jsonFile, std::string &outContent);

    Logger &logger;
};
