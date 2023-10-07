#pragma once

#include "charon/util/class_traits.h"
#include "charon/util/error.h"
#include "charon/util/filesystem.h"

#include <array>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>

struct Time;

enum class LogLevel {
    Error = 1 << 0,
    Info = 1 << 1,
    Warning = 1 << 2,
    Debug = 1 << 3,
    VerboseInfo = 1 << 4,
};
constexpr inline LogLevel operator|(LogLevel left, LogLevel right) {
    return LogLevel(std::underlying_type_t<LogLevel>(left) | std::underlying_type_t<LogLevel>(right));
}
constexpr inline LogLevel operator&(LogLevel left, LogLevel right) {
    return LogLevel(std::underlying_type_t<LogLevel>(left) & std::underlying_type_t<LogLevel>(right));
}
constexpr static inline LogLevel defaultLogLevel = LogLevel::Error | LogLevel::Info | LogLevel::Warning;

struct Logger : NonCopyableAndMovable {
    virtual ~Logger() {}
    virtual void log(LogLevel level, const std::string &message) = 0;
    std::mutex mutex;

    auto raiiSetup() { return RaiiSetup{*this}; }
    static Logger *getInstance() { return instance; }

    struct RaiiSetup {
        RaiiSetup(Logger &logger) : RaiiSetup(&logger) {}
        RaiiSetup(Logger *logger);
        ~RaiiSetup();

    private:
        Logger *previous{};
    };

protected:
    static inline Logger *instance = {};
};

class RaiiLog {
public:
    RaiiLog(LogLevel logLevel, Logger &logger);
    ~RaiiLog();

    template <typename T>
    RaiiLog &operator<<(const T &arg) {
        buffer << arg;
        return *this;
    }

private:
    Logger &logger;
    const LogLevel logLevel;
    std::unique_lock<std::mutex> lock;
    std::ostringstream buffer{};
};

template <>
inline RaiiLog &RaiiLog::operator<< <std::filesystem::path>(const std::filesystem::path &arg) {
#if defined(WIN32)
    // Convert wstring to string while ignoring diacritics
    const std::wstring &src = arg.native();
    const int srcSize = static_cast<int>(src.size() + 1);
    int bytes = WideCharToMultiByte(CP_ACP, 0, src.data(), srcSize, nullptr, 0, nullptr, nullptr);
    FATAL_ERROR_IF(bytes == 0, "WideCharToMultiByte for size checking failed");
    std::string narrowString(bytes - 1, '\0');
    bytes = WideCharToMultiByte(CP_ACP, 0, src.data(), srcSize, narrowString.data(), bytes, nullptr, nullptr);
    FATAL_ERROR_IF(bytes == 0, "WideCharToMultiByte for conversion checking failed");

    std::replace(narrowString.begin(), narrowString.end(), '\\', '/');
    buffer << narrowString;
#else
    buffer << arg.native();
#endif
    return *this;
}

RaiiLog log(LogLevel logLevel, Logger *logger = nullptr);

struct OstreamLogger : Logger {
    OstreamLogger(const Time &time, std::ostream &out, LogLevel allowedLogLevels) : time(time), out(out), allowedLogLevels(allowedLogLevels) {}
    void log(LogLevel level, const std::string &message) override;

private:
    void writeDate();
    void writeLogLevel(LogLevel level);
    const Time &time;
    std::ostream &out;
    LogLevel allowedLogLevels;
};

struct ConsoleLogger : OstreamLogger {
    ConsoleLogger(const Time &time, LogLevel allowedLogLevels) : OstreamLogger(time, std::cout, allowedLogLevels) {}
};

struct FileLogger : OstreamLogger {
    FileLogger(const Time &time, const fs::path &logFile, LogLevel allowedLogLevels);

private:
    std::ofstream file{};
};

struct NullLogger : Logger {
    void log(LogLevel level, const std::string &message) override;
};

struct MultiplexedLogger : Logger {
    template <typename... LoggerTypes>
    MultiplexedLogger(LoggerTypes... args) {
        add(std::forward<LoggerTypes>(args)...);
    }

    template <typename... LoggerTypes>
    void add(LoggerTypes... args) {
        std::array<Logger *, sizeof...(args)> pointers = {args...};
        for (Logger *logger : pointers) {
            this->loggers.push_back(logger);
        }
    }

    void log(LogLevel level, const std::string &message) override {
        for (Logger *logger : this->loggers) {
            logger->log(level, message);
        }
    }

private:
    std::vector<Logger *> loggers = {};
};
