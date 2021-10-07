#pragma once

#include "charon/processor/processor_config.h"

#include <nlohmann/json.hpp>

class ProcessConfigReader {
public:
    bool read(ProcessorConfig &outConfig, const std::filesystem::path &jsonFile);
    bool read(ProcessorConfig &outConfig, const std::string &json);

    const auto &getErrors() const { return outErrors; }

private:
    bool parseProcessorConfig(ProcessorConfig &outConfig, const nlohmann::json &node);
    bool parseProcessorConfigEntry(ProcessorConfigEntry &outConfigEntry, const nlohmann::json &node);
    bool parseProcessorAction(ProcessorAction &outAction, const nlohmann::json &node);

    static std::string readFile(const std::filesystem::path &jsonFile);

    std::vector<std::string> outErrors = {};
};
