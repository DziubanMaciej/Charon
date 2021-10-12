#pragma once

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>

namespace fs = std::filesystem;

enum class LogLevel {
    Error,
    Info,
    Warning,
    Debug,
};

struct Logger {
    virtual ~Logger() {}
    virtual void log(LogLevel level, const std::string &message) = 0;
    std::mutex mutex;

protected:
    const static char *getPreamble(LogLevel level) {
        const static char *preambles[] = {
            "[Error] ",
            "[Info] ",
            "[Warning] ",
            "[Debug] ",
        };
        return preambles[static_cast<size_t>(level)];
    }
};

class RaiiLog {
public:
    RaiiLog(Logger &logger, LogLevel logLevel)
        : logger(logger),
          logLevel(logLevel) {
        lock = std::unique_lock{logger.mutex};
    }

    template <typename T>
    RaiiLog &operator<<(const T &arg) {
        buffer << arg;
        return *this;
    }

    template <>
    RaiiLog &operator<<<std::filesystem::path>(const std::filesystem::path &arg) {
        auto pathString = arg.string();
        std::replace(pathString.begin(), pathString.end(), '\\', '/');
        buffer << pathString;
        return *this;
    }

    ~RaiiLog() {
        logger.log(logLevel, buffer.str());
    }

private:
    const LogLevel logLevel;
    Logger &logger;
    std::unique_lock<std::mutex> lock;
    std::ostringstream buffer{};
};

inline RaiiLog log(Logger &logger, LogLevel logLevel) {
    return RaiiLog{logger, logLevel};
}

struct ConsoleLogger : Logger {
    void log(LogLevel level, const std::string &message) override {
        std::cout << getPreamble(level) << message << '\n';
    }
};

struct FileLogger : Logger {
    FileLogger(const fs::path &logFile) : file(logFile, std::ios::out) {}

    void log(LogLevel level, const std::string &message) override {
        file << getPreamble(level) << message << '\n';
        file.flush();
    }

private:
    std::ofstream file{};
};

struct NullLogger : Logger {
    void log(LogLevel level, const std::string &message) override {}
};

struct MultiplexedLogger : Logger {
    template <typename... LoggerTypes>
    MultiplexedLogger(LoggerTypes... args) {
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
