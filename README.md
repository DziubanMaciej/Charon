# Charon

Charon is a simple C++ application able to watch selected directories for filesystem events (such as file creation) and perform a set actions on related files. Events can be filtered based on file extension. Charon can also assign incrementing numbers to destination filename while copying/moving. The software is configured via a json config file (structure explained in further chapters). Supported platforms: Windows, Linux.

# Example

To illustrate how this works, a simple file sorter was implemented by watching `DumpFolder` directory and filtering events by extension. Related files are then moved into different destination directories based on which filter succeeded. See the [config file](/assets/config.json).
![example usage](/assets/example.gif)

# Modes of operation

Charon can work in one of two modes: as a normal console application and as a daemon - a background process. To run as a daemon, pass `-d` switch to Charon executable. This will cause Charon not to take user's input from console or print logs *stdout*. On Windows the application will also register itself in system tray (taskbar notification area) and can be easily closed from there.

# Json format

The config file is an array of *matcher*  objects. See the [example config file](/assets/config.json).

### Matcher

A matcher object encapsulates a watched directory, which will produce file events, a set of filters for the events and a set of actions to perform when all the filters are matched. Actions are performed in the order in which they are specified in the config file. Example matcher object:

```
{
    "watchedFolder" : "D:/WatchedFolder",
    "extensions" : ["mp4", "avi"],
    "actions" : [
        {
            "type" : "copy",
            "destinationDir" : "D:/Videos",
            "destinationName" : "${name}"
        },
        {
            "type" : "copy",
            "destinationDir" : "E:/VideosBackup",
            "destinationName" : "${name}"
        }
    ]
}
```

### Action

An action object defines filesystem operation to perform on files from matched file events. Every action object must contain a `type` field containing a valid action type and all the required type-specific fields. Supported action types:

- `copy` - copy the file. Additional fields: `destinationDir`, `destinationName`.
- `move` - move the file. Aditional fields: `destinationDir`, `destinationName`.
- `remove` - remove the file.
- `print` - print matched file event to logs.

The `destinationName` for `copy` and `move` actions is without extension. The original extension is preserved. Apart from explicit characters, it can use a number of additional techniques applied during name resolution:

- counters - tells Charon to assign incrementing numbers to filenames and insert them in place of hash characters (`#`). For example `file_##` will cause subsequent copy operations to resolve to following names: `file_00`, `file_01`, `file_02`, and so on. The action will fail, if all the counter values are already taken.
- pseudo-variables - they are substituted with actual values:
  - `${name}` - name of the source file (without extension)
  - `${extension}` - extension of the source file
  - `${previousName}` - destination name resolved by the previous action in current matcher. Result is undefined, when there's no previous action or previous action did not resolve any destination name (e.g. `remove` action).

# Command line arguments

Most of Charon functionality is steered with the config file, but it also accepts a few command line arguments for the most basic configuration

- `--config`, `-c` - sets the config file path.
- `--log`, `-l` - sets the log file path. By default logs are not saved to any file. Each new Charon invocation appends to the existing log file.
- `--daemon`, `-d` - run as daemon (background process)

# Building

To build Charon you will need a working C++17 compiler, git and CMake.

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
