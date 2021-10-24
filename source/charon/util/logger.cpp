#include "charon/util/logger.h"
#include "charon/util/time.h"

Logger::RaiiSetup::RaiiSetup(Logger *logger) {
    previous = instance;
    instance = logger;
}

Logger::RaiiSetup::~RaiiSetup() {
    instance = previous;
}

RaiiLog::RaiiLog(LogLevel logLevel, Logger &logger)
    : logger(logger),
      logLevel(logLevel),
      lock(logger.mutex) {}

RaiiLog::~RaiiLog() {
    logger.log(logLevel, buffer.str());
}

RaiiLog log(LogLevel logLevel, Logger *logger) {
    if (logger == nullptr) {
        logger = Logger::getInstance();
        FATAL_ERROR_IF(logger == nullptr, "No logger instance set");
    }
    return RaiiLog{logLevel, *logger};
}

void OstreamLogger::log(LogLevel level, const std::string &message) {
    writeDate();
    writeLogLevel(level);
    out << ' ' << message << std::endl;
}

void OstreamLogger::writeDate() {
    out << '[';
    time.writeCurrentTime(out);
    out << ']';
}

void OstreamLogger::writeLogLevel(LogLevel level) {
    const static char *preambles[] = {
        "[Error]",
        "[Info]",
        "[Warning]",
        "[Debug]",
    };
    out << preambles[static_cast<size_t>(level)];
}

FileLogger::FileLogger(const Time &time, const fs::path &logFile)
    : OstreamLogger(time, file),
      file(logFile, std::ios::app) {}

void NullLogger::log([[maybe_unused]] LogLevel level, [[maybe_unused]] const std::string &message) {}
