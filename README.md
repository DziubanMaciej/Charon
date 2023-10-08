# Charon
*Charon* is a utility application designed to help categorize and/or backup files. It is able to perform a set actions on files either after specifying them manually or after detecting filesystem events, such as file creation. *Charon* is configured via a json config file (see [format docs](/docs/JsonFormat.md) for details). Supported platforms are Windows and Linux (tested on Ubuntu).



# Example
To illustrate how *Charon* can be used, a simple file categorizer was implemented by watching `DumpFolder` directory and filtering events by extension. Related files are then moved into different destination directories based on which filter they match. See the [config file](/docs/example_config_default.json).
![example usage](/docs/example.gif)



# Console mode and daemon mode
By default *Charon* works as a console application. It blocks the console and prints its logs to *stdout*. *Charon* running as console application can be explicitly killed by `ctrl+C` or by inputting `q` to *stdin* (in watch mode).

*Charon* can also work as a daemon - a background process. In this mode it does not block the console. Logs are not printed to *stdout*, so it is recommended to specify a log file with `-l` option. To run as a daemon, pass `-d` option to *Charon* executable. On Windows the application also registers itself in system tray (taskbar notification area) and user is able to easily close it from there.



# Watch mode and immediate mode
By default *Charon* works in watch mode. This means it watches for filesystem events, applies filters on them and performs a set of actions. In this mode *Charon* runs indefinitely until manually stopped.

Another mode, that can be selected with `-i` argument, is immediate mode. In this mode the user explictly specifies a set of files to work on. *Charon* performs a set of actions on these files and exits immediately after.

Keep in mind that the json config has a different format depending on used mode. See [format docs](/docs/JsonFormat.md) for details.



# Command line arguments
Most of Charon functionality is steered with the config file, but it also accepts a few command line arguments for the most basic configuration
- `--config`, `-c` - set the config file path.
- `--log`, `-l` - set the log file path. By default logs are not saved to any file. Each new *Charon* invocation appends to the existing log file.
- `--verbose`, `-v` - produce extended logs.
- `--immediate`, `-i` - work in immediate mode.
- `--immediate-files`, `-f` - specify file to process in immediate mode.
- `--daemon`, `-d` - run as a daemon (background process).



# Building
To build *Charon* a working C++17 compiler, git and CMake are required.
```
git clone https://github.com/DziubanMaciej/Charon
mkdir build -p
cd build
cmake ..
cmake --build . --config Release
```

To run all the tests, you wil additionally need Python 3.6 or newer (end-to-end acceptance tests are written in Python)

```
ctest -C Release --verbose
```
