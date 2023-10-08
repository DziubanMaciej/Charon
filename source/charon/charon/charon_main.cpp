#include "charon/charon/charon.h"
#include "charon/processor/processor_config_reader.h"
#include "charon/processor/processor_config_validator.h"
#include "charon/user_interface/console_user_interface.h"
#include "charon/user_interface/daemon_user_interface.h"
#include "charon/util/argument_parser.h"
#include "charon/util/filesystem_impl.h"
#include "charon/util/logger.h"
#include "charon/util/time.h"
#include "charon/watcher/directory_watcher_factory.h"

int charonMain(int argc, char **argv, bool isDaemon) {
    ArgumentParser argParser{argc, argv};
    const fs::path logPath = argParser.getArgumentValue<fs::path>(ArgNames{"-l", "--log"}, {});
    const fs::path configPath = argParser.getArgumentValue<fs::path>(ArgNames{"-c", "--config"}, fs::current_path() / "config.json");
    const bool verbose = argParser.getArgumentValue<bool>(ArgNames{"-v", "--verbose"}, false);
    const bool isImmediateMode = argParser.getArgumentValue<bool>(ArgNames{"-i", "--immediate"}, false);
    const std::vector<fs::path> immediateModePaths = argParser.getArgumentValues<fs::path>(ArgNames{"-f", "--immediate-files"});

    // Setup logger
    LogLevel allowedLogLevels = defaultLogLevel;
    if (verbose) {
        allowedLogLevels = allowedLogLevels | LogLevel::VerboseInfo;
    }
    TimeImpl time{};
    FileLogger fileLogger{time, logPath, allowedLogLevels};
    ConsoleLogger consoleLogger{time, allowedLogLevels};
    MultiplexedLogger logger{};
    if (!isDaemon) {
        logger.add(&consoleLogger);
    }
    if (!logPath.empty()) {
        logger.add(&fileLogger);
    }
    const auto loggerSetup = logger.raiiSetup();

    // Log arguments
    log(LogLevel::Info) << "";
    log(LogLevel::Info) << "Arguments:";
    log(LogLevel::Info) << "    logPath = " << logPath;
    log(LogLevel::Info) << "    configPath = " << configPath;
    log(LogLevel::Info) << "    isDaemon = " << isDaemon;
    log(LogLevel::Info) << "    isImmediateMode = " << isImmediateMode;
    if (isImmediateMode) {
        auto logLine = log(LogLevel::Info);
        logLine << "    immediateModePaths = {";
        for (size_t i = 0u; i < immediateModePaths.size(); i++) {
            logLine << immediateModePaths[i];
            if (i < immediateModePaths.size() - 1) {
                logLine << ", ";
            }
        }
        logLine << "}";
    }

    // Validate arguments
    if (isDaemon && isImmediateMode) {
        // Technically we could, but it doesn't really make sense, so let's disallow it.
        log(LogLevel::Error) << "Cannot run as daemon in immediate mode. Exiting.";
        return EXIT_FAILURE;
    }
    if (isImmediateMode && immediateModePaths.empty()) {
        log(LogLevel::Warning) << "No files specified in immediate mode. Exiting.";
        return EXIT_SUCCESS;
    }

    // Read config
    ProcessConfigReader reader{};
    ProcessorConfig config{};
    ProcessorConfig::Type processorConfigType = ProcessorConfig::Type::Matchers;
    if (isImmediateMode) {
        processorConfigType = ProcessorConfig::Type::Actions;
    }
    const bool success = reader.read(config, configPath, processorConfigType);
    if (!success) {
        return EXIT_FAILURE;
    }
    if (!ProcessorConfigValidator::validateConfig(config)) {
        return EXIT_FAILURE;
    }

    // Run Charon
    FilesystemImpl filesystem{};
    DirectoryWatcherFactoryImpl watcherFactory{};
    Charon charon{config, filesystem, watcherFactory};
    charon.setLogFilePath(logPath);
    charon.setConfigFilePath(configPath);
    if (!charon.start()) {
        log(LogLevel::Error) << "Error starting Charon.";
        return EXIT_FAILURE;
    }

    if (isImmediateMode) {
        // Add files to be processed by Charon
        charon.processImmediate(immediateModePaths);
        if (!charon.stop()) {
            log(LogLevel::Error) << "Error stopping Charon.";
            return EXIT_FAILURE;
        }
    } else {
        // Run user interface
        std::unique_ptr<UserInterface> userInterface{};
        if (isDaemon) {
            userInterface = DaemonUserInterface::create(charon);
        } else {
            userInterface = std::make_unique<ConsoleUserInterface>(charon);
        }
        userInterface->run();
    }

    return EXIT_SUCCESS;
}
