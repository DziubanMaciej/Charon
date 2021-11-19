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
    Error,
    Info,
    Warning,
    Debug,
};

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

#if defined(WIN32)
    template <>
    RaiiLog &operator<<<std::filesystem::path>(const std::filesystem::path &arg) {
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
        return *this;
    }
#endif

private:
    Logger &logger;
    const LogLevel logLevel;
    std::unique_lock<std::mutex> lock;
    std::ostringstream buffer{};
};

RaiiLog log(LogLevel logLevel, Logger *logger = nullptr);

struct OstreamLogger : Logger {
    OstreamLogger(const Time &time, std::ostream &out) : time(time), out(out) {}
    void log(LogLevel level, const std::string &message) override;

private:
    void writeDate();
    void writeLogLevel(LogLevel level);
    const Time &time;
    std::ostream &out;
};

struct ConsoleLogger : OstreamLogger {
    ConsoleLogger(const Time &time) : OstreamLogger(time, std::cout) {}
};

struct FileLogger : OstreamLogger {
    FileLogger(const Time &time, const fs::path &logFile);

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
