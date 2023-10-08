#include "processor_config_reader.h"

#include "charon/processor/processor_config.h"
#include "charon/util/error.h"
#include "charon/util/logger.h"

#include <fstream>
#include <sstream>

bool ProcessConfigReader::read(ProcessorConfig &outConfig, const std::filesystem::path &jsonFile, ProcessorConfig::Type type) {
    std::string json{};
    if (!readFile(jsonFile, json)) {
        log(LogLevel::Error) << "Could not read config file";
        return false;
    }
    return read(outConfig, json, type);
}

bool ProcessConfigReader::read(ProcessorConfig &outConfig, const std::string &json, ProcessorConfig::Type type) {
    const nlohmann::json rootNode = nlohmann::json::parse(json, nullptr, false);
    if (rootNode.is_discarded()) {
        log(LogLevel::Error) << "Specified json was badly formed.";
        return false;
    }
    switch (type) {
    case ProcessorConfig::Type::Matchers:
        return parseProcessorConfigMatchers(outConfig, rootNode);
    case ProcessorConfig::Type::Actions:
        return parseProcessorConfigActions(outConfig, rootNode);
    default:
        FATAL_ERROR("Invalid processor config type");
    }
}

bool ProcessConfigReader::readFile(const std::filesystem::path &jsonFile, std::string &outContent) {
    std::ifstream stream{jsonFile};
    if (!stream) {
        return false;
    }

    std::stringstream buffer;
    buffer << stream.rdbuf();
    outContent = buffer.str();
    return true;
}

bool ProcessConfigReader::parseProcessorConfigMatchers(ProcessorConfig &outConfig, const nlohmann::json &node) {
    if (!node.is_array()) {
        log(LogLevel::Error) << "Root node must be an array";
        return false;
    }

    ProcessorConfig::Matchers &configData = outConfig.createMatchers();
    for (const nlohmann::json &matcherNode : node) {
        ProcessorActionMatcher matcher{};
        if (parseProcessorActionMatcher(matcher, matcherNode)) {
            configData.matchers.push_back(matcher);
        } else {
            return false;
        }
    }

    return true;
}

bool ProcessConfigReader::parseProcessorConfigActions(ProcessorConfig &outConfig, const nlohmann::json &node) {
    ProcessorConfig::Actions &configData = outConfig.createActions();
    return parseProcessorActions(configData.actions, node);
}

bool ProcessConfigReader::parseProcessorActionMatcher(ProcessorActionMatcher &outActionMatcher, const nlohmann::json &node) {
    if (!node.is_object()) {
        log(LogLevel::Error) << "Action matcher node must be an object";
        return false;
    }

    if (auto it = node.find("watchedFolder"); it != node.end()) {
        outActionMatcher.watchedFolder = it->get<std::string>();
    } else {
        log(LogLevel::Error) << "Action matcher node must contain \"watchedFolder\" field.";
        return false;
    }

    if (auto it = node.find("extensions"); it != node.end()) {
        if (!it->is_array()) {
            log(LogLevel::Error) << "Action matcher \"extensions\" member must be an array.";
            return false;
        }
        auto extensions = it->get<std::vector<std::string>>();
        for (auto &extension : extensions) {
            outActionMatcher.watchedExtensions.emplace_back(extension);
        }
    }

    if (auto it = node.find("actions"); it != node.end()) {
        return parseProcessorActions(outActionMatcher.actions, *it);
    } else {
        log(LogLevel::Error) << "Action matcher node must contain \"actions\" field.";
        return false;
    }
}

NLOHMANN_JSON_SERIALIZE_ENUM(ProcessorAction::Type,
                             {
                                 {ProcessorAction::Type::Invalid, nullptr},
                                 {ProcessorAction::Type::Copy, "copy"},
                                 {ProcessorAction::Type::Move, "move"},
                                 {ProcessorAction::Type::Remove, "remove"},
                                 {ProcessorAction::Type::Print, "print"},
                             })

bool ProcessConfigReader::parseProcessorActions(std::vector<ProcessorAction> &outActions, const nlohmann::json &node) {
    if (!node.is_array()) {
        log(LogLevel::Error) << "Actions list must be an array.";
        return false;
    }

    for (const nlohmann::json &actionNode : node) {
        ProcessorAction action{};
        if (parseProcessorAction(action, actionNode)) {
            outActions.push_back(action);
        } else {
            return false;
        }
    }
    return true;
}

bool ProcessConfigReader::parseProcessorAction(ProcessorAction &outAction, const nlohmann::json &node) {
    if (!node.is_object()) {
        log(LogLevel::Error) << "Action node must be an object";
        return false;
    }

    if (auto it = node.find("type"); it != node.end()) {
        outAction.type = it->get<ProcessorAction::Type>();
        if (outAction.type == ProcessorAction::Type::Invalid) {
            log(LogLevel::Error) << std::string{"Action node \"type\" field has an invalid value: "} + it->get<std::string>();
            return false;
        }
    } else {
        log(LogLevel::Error) << "Action node must contain \"type\" field.";
        return false;
    }

    switch (outAction.type) {
    case ProcessorAction::Type::Copy:
    case ProcessorAction::Type::Move: {
        ProcessorAction::MoveOrCopy data{};
        if (auto it = node.find("destinationDir"); it != node.end()) {
            data.destinationDir = it->get<std::string>();
        } else {
            log(LogLevel::Error) << "Action node of type \"copy\" or \"move\" must contain \"destinationDir\" field.";
            return false;
        }

        if (auto it = node.find("destinationName"); it != node.end()) {
            data.destinationName = it->get<std::string>();
        } else {
            log(LogLevel::Error) << "Action node of type \"copy\" or \"move\" must contain \"destinationName\" field.";
            return false;
        }

        outAction.data = data;
        break;
    }
    case ProcessorAction::Type::Remove:
        outAction.data = ProcessorAction::Remove{};
        break;
    case ProcessorAction::Type::Print:
        outAction.data = ProcessorAction::Print{};
        break;
    default:
        UNREACHABLE_CODE;
    }

    return true;
}
