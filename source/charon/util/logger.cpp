#include "charon/util/logger.h"

Logger::RaiiSetup::RaiiSetup(Logger *logger) {
    previous = instance;
    instance = logger;
}

Logger::RaiiSetup::~RaiiSetup() {
    instance = previous;
}

const char *Logger::getPreamble(LogLevel level) {
    const static char *preambles[] = {
        "[Error] ",
        "[Info] ",
        "[Warning] ",
        "[Debug] ",
    };
    return preambles[static_cast<size_t>(level)];
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
    out << getPreamble(level) << message << '\n';
    out.flush();
}

FileLogger::FileLogger(const fs::path &logFile)
    : OstreamLogger(file),
      file(logFile, std::ios::out) {}

void NullLogger::log([[maybe_unused]] LogLevel level, [[maybe_unused]] const std::string &message) {}
