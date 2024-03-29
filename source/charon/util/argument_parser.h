#pragma once

#include "charon/util/filesystem.h"

#include <algorithm>
#include <codecvt>
#include <locale>
#include <sstream>
#include <string>
#include <vector>

using ArgName = std::string;
using ArgNames = std::vector<ArgName>;

class ArgumentParser {
    template <typename T>
    using ConvertFunction = T (*)(const std::string &);

public:
    ArgumentParser(int argc, char **argv) {
        for (int i = 1; i < argc; i++) {
            this->args.push_back(argv[i]);
        }
    }

    // Helpers for single-name arguments
    template <typename ResultType>
    ResultType getArgumentValue(const ArgName &name, const ResultType &defaultValue) {
        return getArgumentValue<ResultType>(ArgNames{name}, defaultValue);
    }
    template <typename ResultType>
    std::vector<ResultType> getArgumentValues(const ArgName &name) {
        return getArgumentValues<ResultType>(ArgNames{name});
    }

    // Lookup methods, varying implementation for different types
    template <typename ResultType>
    ResultType getArgumentValue(const ArgNames &names, const ResultType &defaultValue);
    template <typename ResultType>
    std::vector<ResultType> getArgumentValues(const ArgNames &names);

    // Manipulation of the arguments
    void removeArgs(const ArgNames &names) {
        const auto isForRemoval = [names](const std::string &s) {
            return std::find(names.begin(), names.end(), s) != names.end();
        };
        args.erase(std::remove_if(args.begin(), args.end(), isForRemoval));
    }
    std::string getCommandLine() const {
        std::ostringstream result{};
        for (auto i = 0u; i < args.size(); i++) {
            result << args[i];
            if (i < args.size() - 1) {
                result << ' ';
            }
        }
        return result.str();
    }

private:
    // Conversion from string to various types

    template <typename ResultType>
    static ResultType convertFunction(const std::string &arg) {
        if constexpr (std::is_same_v<ResultType, wchar_t>) {
            return static_cast<ResultType>(arg[0]);
        }
        if constexpr (std::is_integral_v<ResultType>) {
            return static_cast<ResultType>(std::stoll(arg));
        }
        if constexpr (std::is_same_v<ResultType, std::string>) {
            return arg;
        }
        if constexpr (std::is_same_v<ResultType, std::wstring>) {
            std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
            return converter.from_bytes(arg);
        }
        if constexpr (std::is_same_v<ResultType, std::filesystem::path>) {
            return std::filesystem::path(arg);
        }
    }

    // Arguments captures
    std::vector<std::string> args;
};

template <>
inline bool ArgumentParser::getArgumentValue<bool>(const std::vector<std::string> &names, const bool &defaultValue) {
    for (auto &name : names) {
        auto nameIt = std::find(this->args.begin(), this->args.end(), name);
        if (nameIt != this->args.end()) {
            return true;
        }
    }
    return defaultValue;
}

template <typename ResultType>
inline ResultType ArgumentParser::getArgumentValue(const std::vector<std::string> &names, const ResultType &defaultValue) {
    for (auto &name : names) {
        auto nameIt = std::find(this->args.begin(), this->args.end(), name);
        if (nameIt == this->args.end())
            continue;

        auto valueIt = nameIt + 1;
        if (valueIt == this->args.end())
            continue;

        return ArgumentParser::convertFunction<ResultType>(*valueIt);
    }
    return defaultValue;
}

template <typename ResultType>
inline std::vector<ResultType> ArgumentParser::getArgumentValues(const std::vector<std::string> &names) {
    // Find iterator to one of the names for this argument
    auto nameIt = this->args.end();
    for (auto &name : names) {
        nameIt = std::find(this->args.begin(), this->args.end(), name);
        if (nameIt != this->args.end()) {
            break;
        }
    }
    if (nameIt == this->args.end()) {
        return {};
    }

    // Append values
    std::vector<ResultType> result{};
    for (auto valueIt = nameIt + 1; valueIt != this->args.end(); valueIt++) {
        if (valueIt == this->args.end()) {
            break;
        }
        const std::string &value = *valueIt;
        if (value.empty() || value[0] == '-') {
            break;
        }

        result.push_back(ArgumentParser::convertFunction<ResultType>(value));
    }
    return result;
}
