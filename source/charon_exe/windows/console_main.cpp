#include "charon/charon/charon_main.h"
#include "charon/util/argument_parser.h"

#include <Windows.h>
#include <filesystem>

int runBackgroundProcess(const std::filesystem::path &exeName, std::string &commandLine) {
    SECURITY_ATTRIBUTES security_attributes;
    security_attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
    security_attributes.bInheritHandle = TRUE;
    security_attributes.lpSecurityDescriptor = nullptr;

    STARTUPINFO startup_info = {};
    startup_info.cb = sizeof(STARTUPINFO);

    std::ostringstream fullCommandLineStream;
    fullCommandLineStream << exeName << ' ' << commandLine;
    std::string fullCommandLine = fullCommandLineStream.str();

    PROCESS_INFORMATION process_info = {};
    BOOL success = CreateProcessA(
        exeName.string().c_str(),
        &fullCommandLine[0],
        nullptr,
        nullptr,
        false,
        0,
        nullptr,
        nullptr,
        &startup_info,
        &process_info);

    CloseHandle(process_info.hProcess);
    CloseHandle(process_info.hThread);

    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

int main(int argc, char **argv) {
    ArgumentParser argParser{argc, argv};
    ArgNames daemonNames{"-d", "--daemon"};
    const bool runAsDaemon = argParser.getArgumentValue<bool>(daemonNames, false);
    if (runAsDaemon) {
        // If we decide we want to run in background, it is already too late for this process.
        // We have to run new process, don't wait for it and terminate. New process will be
        // orphaned, but will continue to run normally. Because it'll be Win32 application, it
        // will have no visible console window and will run truly in background.

        // Remove -d argument from command line. Daemon executable does not support it
        argParser.removeArgs(daemonNames);
        std::string newArgs = argParser.getCommandLine();

        // Get path for the daemon executable.
        WCHAR path[MAX_PATH];
        GetModuleFileNameW(NULL, path, MAX_PATH);
        const auto daemonExePath = std::filesystem::path{path}.parent_path() / "CharonDaemon.exe";

        // Run the Win32 process
        return runBackgroundProcess(daemonExePath, newArgs);
    }

    // Here we select to work as a normal console application
    return charonMain(argc, argv, false);
}
