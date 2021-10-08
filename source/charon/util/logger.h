#pragma once

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
    virtual void log(const std::string &message) = 0;
    std::mutex mutex;
};

class RaiiLog {
public:
    RaiiLog(Logger *logger, LogLevel logLevel) : logger(logger) {
        if (logger) {
            lock = std::unique_lock{logger->mutex};
        }

        const static char *preambles[] = {
            "[Error]",
            "[Info]",
            "[Warning]",
            "[Debug]",
        };
        buffer << preambles[static_cast<size_t>(logLevel)] << ' ';
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
        buffer << '\n';
        if (logger) {
            logger->log(buffer.str());
        }
    }

private:
    Logger *logger;
    std::unique_lock<std::mutex> lock;
    std::ostringstream buffer{};
};

inline RaiiLog log(Logger *logger, LogLevel logLevel) {
    return RaiiLog{logger, logLevel};
}

struct ConsoleLogger : Logger {
    void log(const std::string &message) override {
        std::cout << message;
    }
};

struct NullLogger : Logger {
    void log(const std::string &message) override {}
};
