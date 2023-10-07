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
    if ((level & allowedLogLevels) == LogLevel(0)) {
        return;
    }

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
    const char *preamble = nullptr;
    switch (level) {
    case LogLevel::Error:
        preamble = "[Error]";
        break;
    case LogLevel::Info:
        preamble = "[Info]";
        break;
    case LogLevel::Warning:
        preamble = "[Warning]";
        break;
    case LogLevel::Debug:
        preamble = "[Debug]";
        break;
    case LogLevel::VerboseInfo:
        preamble = "[VerboseInfo]";
        break;
    default:
        preamble = "[Unknown]";
        break;
    }
    out << preamble;
}

FileLogger::FileLogger(const Time &time, const fs::path &logFile, LogLevel allowedLogLevels)
    : OstreamLogger(time, file, allowedLogLevels),
      file(logFile, std::ios::app) {}

void NullLogger::log([[maybe_unused]] LogLevel level, [[maybe_unused]] const std::string &message) {}
