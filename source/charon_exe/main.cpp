#include "charon/charon/charon.h"
#include "charon/processor/processor_config_reader.h"
#include "charon/user_interface/console_user_interface.h"
#include "charon/user_interface/daemon_user_interface.h"
#include "charon/util/argument_parser.h"
#include "charon/util/filesystem_impl.h"
#include "charon/util/logger.h"
#include "charon/watcher/directory_watcher_factory.h"

#include <iostream>

int main(int argc, char **argv) {
    ArgumentParser argParser{argc, argv};
    const fs::path logPath = argParser.getArgumentValue<fs::path>(ArgNames{"-l", "--log"}, fs::current_path() / "log.txt");
    const fs::path configPath = argParser.getArgumentValue<fs::path>(ArgNames{"-c", "--config"}, fs::current_path() / "config.json");
    const bool runAsDaemon = argParser.getArgumentValue<bool>(ArgNames{"-d", "--daemon"}, false);

    // Setup logger
    FileLogger fileLogger{logPath};
    ConsoleLogger consoleLogger{};
    MultiplexedLogger logger{&fileLogger};
    if (!runAsDaemon) {
        logger.add(&consoleLogger);
    }

    // Read config
    ProcessConfigReader reader{logger};
    ProcessorConfig config{};
    const bool success = reader.read(config, configPath);
    if (!success) {
        return EXIT_FAILURE;
    }

    // Run Charon
    FilesystemImpl filesystem{};
    DirectoryWatcherFactoryImpl watcherFactory{};
    Charon charon{config, filesystem, logger, watcherFactory};
    charon.setLogFilePath(logPath);
    charon.setConfigFilePath(configPath);
    if (!charon.start()) {
        return EXIT_FAILURE;
    }

    // Run user interface
    std::unique_ptr<UserInterface> userInterface{};
    if (runAsDaemon) {
        userInterface = DaemonUserInterface::create(charon);
    } else {
        userInterface = std::make_unique<ConsoleUserInterface>(charon);
    }
    userInterface->run();
}
