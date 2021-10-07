#include "processor_config_reader.h"

#include "charon/processor/processor_config.h"
#include "charon/util/error.h"

#include <fstream>
#include <sstream>

bool ProcessConfigReader::read(ProcessorConfig &outConfig, const std::filesystem::path &jsonFile) {
    return read(outConfig, readFile(jsonFile));
}

bool ProcessConfigReader::read(ProcessorConfig &outConfig, const std::string &json) {
    outErrors.clear();

    const nlohmann::json rootNode = nlohmann::json::parse(json, nullptr, false);
    if (rootNode.is_discarded()) {
        outErrors.push_back("Specified json was badly formed.");
        return false;
    }
    return parseProcessorConfig(outConfig, rootNode);
}

std::string ProcessConfigReader::readFile(const std::filesystem::path &jsonFile) {
    std::ifstream stream{jsonFile};
    std::stringstream buffer;
    buffer << stream.rdbuf();
    return buffer.str();
}

bool ProcessConfigReader::parseProcessorConfig(ProcessorConfig &outConfig, const nlohmann::json &node) {
    if (!node.is_array()) {
        outErrors.push_back("Root node must be an array");
        return false;
    }

    for (const nlohmann::json &entryNode : node) {
        ProcessorConfigEntry entry{};
        if (parseProcessorConfigEntry(entry, entryNode)) {
            outConfig.entries.push_back(entry);
        } else {
            return false;
        }
    }

    return true;
}

bool ProcessConfigReader::parseProcessorConfigEntry(ProcessorConfigEntry &outConfigEntry, const nlohmann::json &node) {
    if (!node.is_object()) {
        outErrors.push_back("Config entry node must be an object");
        return false;
    }

    if (auto it = node.find("watchedFolder"); it != node.end()) {
        outConfigEntry.watchedFolder = it->get<std::string>();
    } else {
        outErrors.push_back("Config entry node must contain \"watchedFolder\" field.");
        return false;
    }

    if (auto it = node.find("extensions"); it != node.end()) {
        if (!it->is_array()) {
            outErrors.push_back("Config entry \"extensions\" member must be an array.");
            return false;
        }
        outConfigEntry.watchedExtensions = it->get<std::vector<std::string>>();
    }

    if (auto it = node.find("actions"); it != node.end()) {
        if (!it->is_array()) {
            outErrors.push_back("Config entry \"actions\" member must be an array.");
            return false;
        }

        for (const nlohmann::json &actionNode : *it) {
            ProcessorAction action{};
            if (parseProcessorAction(action, actionNode)) {
                outConfigEntry.actions.push_back(action);
            } else {
                return false;
            }
        }
    } else {
        outErrors.push_back("Config entry node must contain \"actions\" field.");
        return false;
    }

    return true;
}

NLOHMANN_JSON_SERIALIZE_ENUM(ProcessorAction::Type,
                             {
                                 {ProcessorAction::Type::Invalid, nullptr},
                                 {ProcessorAction::Type::Copy, "copy"},
                                 {ProcessorAction::Type::Move, "move"},
                                 {ProcessorAction::Type::Remove, "remove"},
                             })

bool ProcessConfigReader::parseProcessorAction(ProcessorAction &outAction, const nlohmann::json &node) {
    if (!node.is_object()) {
        outErrors.push_back("Action node must be an object");
        return false;
    }

    if (auto it = node.find("type"); it != node.end()) {
        outAction.type = it->get<ProcessorAction::Type>();
        if (outAction.type == ProcessorAction::Type::Invalid) {
            outErrors.push_back(std::string{"Action node \"type\" field has an invalid value: "} + it->get<std::string>());
            return false;
        }
    } else {
        outErrors.push_back("Action node must contain \"type\" field.");
        return false;
    }

    switch (outAction.type) {
    case ProcessorAction::Type::Copy:
    case ProcessorAction::Type::Move: {
        ProcessorAction::MoveOrCopy data{};
        if (auto it = node.find("destinationDir"); it != node.end()) {
            data.destinationDir = it->get<std::string>();
        } else {
            outErrors.push_back("Action node of type \"copy\" or \"move\" must contain \"destinationDir\" field.");
            return false;
        }

        if (auto it = node.find("destinationName"); it != node.end()) {
            data.destinationName = it->get<std::string>();
        } else {
            outErrors.push_back("Action node of type \"copy\" or \"move\" must contain \"destinationName\" field.");
            return false;
        }

        if (auto it = node.find("overwriteExisting"); it != node.end()) {
            data.overwriteExisting = it->get<bool>();
        }

        outAction.data = data;
        break;
    }
    case ProcessorAction::Type::Remove:
        outAction.data = ProcessorAction::Remove{};
        break;
    default:
        UNREACHABLE_CODE;
    }

    return true;
}
