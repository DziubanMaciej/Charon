#include "processor_config_reader.h"

#include "charon/processor/processor_config.h"
#include "charon/util/error.h"
#include "charon/util/logger.h"

#include <fstream>
#include <sstream>

ProcessConfigReader::ProcessConfigReader(Logger &logger)
    : logger(logger) {}

bool ProcessConfigReader::read(ProcessorConfig &outConfig, const std::filesystem::path &jsonFile) {
    return read(outConfig, readFile(jsonFile));
}

bool ProcessConfigReader::read(ProcessorConfig &outConfig, const std::string &json) {
    const nlohmann::json rootNode = nlohmann::json::parse(json, nullptr, false);
    if (rootNode.is_discarded()) {
        log(logger, LogLevel::Error) << "Specified json was badly formed.";
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
        log(logger, LogLevel::Error) << "Root node must be an array";
        return false;
    }

    for (const nlohmann::json &matcherNode : node) {
        ProcessorActionMatcher matcher{};
        if (parseProcessorActionMatcher(matcher, matcherNode)) {
            outConfig.matchers.push_back(matcher);
        } else {
            return false;
        }
    }

    return true;
}

bool ProcessConfigReader::parseProcessorActionMatcher(ProcessorActionMatcher &outActionMatcher, const nlohmann::json &node) {
    if (!node.is_object()) {
        log(logger, LogLevel::Error) << "Action matcher node must be an object";
        return false;
    }

    if (auto it = node.find("watchedFolder"); it != node.end()) {
        outActionMatcher.watchedFolder = it->get<std::string>();
    } else {
        log(logger, LogLevel::Error) << "Action matcher node must contain \"watchedFolder\" field.";
        return false;
    }

    if (auto it = node.find("extensions"); it != node.end()) {
        if (!it->is_array()) {
            log(logger, LogLevel::Error) << "Action matcher \"extensions\" member must be an array.";
            return false;
        }
        outActionMatcher.watchedExtensions = it->get<std::vector<std::string>>();
    }

    if (auto it = node.find("actions"); it != node.end()) {
        if (!it->is_array()) {
            log(logger, LogLevel::Error) << "Action matcher \"actions\" member must be an array.";
            return false;
        }

        for (const nlohmann::json &actionNode : *it) {
            ProcessorAction action{};
            if (parseProcessorAction(action, actionNode)) {
                outActionMatcher.actions.push_back(action);
            } else {
                return false;
            }
        }
    } else {
        log(logger, LogLevel::Error) << "Action matcher node must contain \"actions\" field.";
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
                                 {ProcessorAction::Type::Print, "print"},
                             })

bool ProcessConfigReader::parseProcessorAction(ProcessorAction &outAction, const nlohmann::json &node) {
    if (!node.is_object()) {
        log(logger, LogLevel::Error) << "Action node must be an object";
        return false;
    }

    if (auto it = node.find("type"); it != node.end()) {
        outAction.type = it->get<ProcessorAction::Type>();
        if (outAction.type == ProcessorAction::Type::Invalid) {
            log(logger, LogLevel::Error) << std::string{"Action node \"type\" field has an invalid value: "} + it->get<std::string>();
            return false;
        }
    } else {
        log(logger, LogLevel::Error) << "Action node must contain \"type\" field.";
        return false;
    }

    switch (outAction.type) {
    case ProcessorAction::Type::Copy:
    case ProcessorAction::Type::Move: {
        ProcessorAction::MoveOrCopy data{};
        if (auto it = node.find("destinationDir"); it != node.end()) {
            data.destinationDir = it->get<std::string>();
        } else {
            log(logger, LogLevel::Error) << "Action node of type \"copy\" or \"move\" must contain \"destinationDir\" field.";
            return false;
        }

        if (auto it = node.find("destinationName"); it != node.end()) {
            data.destinationName = it->get<std::string>();
        } else {
            log(logger, LogLevel::Error) << "Action node of type \"copy\" or \"move\" must contain \"destinationName\" field.";
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
    case ProcessorAction::Type::Print:
        outAction.data = ProcessorAction::Print{};
        break;
    default:
        UNREACHABLE_CODE;
    }

    return true;
}
