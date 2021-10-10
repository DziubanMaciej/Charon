#pragma once

#include <filesystem>
#include <iostream>
#include <mutex>
#include <sstream>

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
            "[Info]",
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
        std::cout << getPreamble(level) << ' ' << message << '\n';
    }
};

struct NullLogger : Logger {
    void log(LogLevel level, const std::string &message) override {}
};
