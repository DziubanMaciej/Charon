#pragma once

#include "charon/processor/processor_config.h"

#include <nlohmann/json.hpp>

struct Logger;

class ProcessConfigReader {
public:
    ProcessConfigReader(Logger &logger);

    bool read(ProcessorConfig &outConfig, const std::filesystem::path &jsonFile);
    bool read(ProcessorConfig &outConfig, const std::string &json);

private:
    bool parseProcessorConfig(ProcessorConfig &outConfig, const nlohmann::json &node);
    bool parseProcessorActionMatcher(ProcessorActionMatcher &outActionMatcher, const nlohmann::json &node);
    bool parseProcessorAction(ProcessorAction &outAction, const nlohmann::json &node);

    static std::string readFile(const std::filesystem::path &jsonFile);

    Logger &logger;
};
