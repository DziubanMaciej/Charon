## Objects
First we'll describe json objects defined for the purpose of *Charon* config.



### Action
An action object defines a filesystem operation to perform on files. Every action object must contain a `type` field containing a valid action type and all the required type-specific fields. Supported action types:
- `copy` - copy the file. Required fields: `destinationDir`, `destinationName`. Destination name should be written without extension. The original extension will be preserved. Optional fields: `counterStart`.
- `move` - move the file. Required fields: `destinationDir`, `destinationName`. Destination name should be written without extension. The original extension will be preserved. Optional fields: `counterStart`.
- `remove` - remove the file.
- `print` - print matched file event to logs.

An example of action, which copies a file to `D:\Deskop` directory and names it `foo`.
```json
{
    "type": "copy",
    "destinationDir": "D:/Desktop",
    "destinationName": "foo"
}
```

The `destinationName` for `copy` and `move` can leverage a functionality named counters. Counters tell Charon to assign incrementing numbers to filenames and insert them in place of hash characters (`#`). For example, below action will cause subsequent copy operations to resolve to following names: `foo_00`, `foo_01`, `foo_02`, and so on. The action will fail, if all the counter values are already taken. Optional field `counterStart` can also be specified to offset first counter. Default value is 0.
```json
{
    "type": "copy",
    "destinationDir": "D:/Desktop",
    "destinationName": "foo_##",
    "counterStart": 0
}
```

The `destinationName` for `copy` and `move` can also make use of pseudo-variables. Pseudo-variables are substituted with actual values before executing the action. Supported pseudo-variables:
  - `${name}` - name of the source file (without extension)
  - `${extension}` - extension of the source file
  - `${previousName}` - destination name resolved by the previous action in current matcher. Result is undefined, when there's no previous action or previous action did not resolve any destination name (e.g. `remove` action).



### Matcher
A matcher object encapsulates a watched directory, which will produce file events, a set of filters for the events and a set of actions to perform when filters are matched. Actions are performed in the order in which they are specified in the config file. Example matcher object:
```json
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



## Config
The complete *Charon* config can be in one of two forms. Config format depends of *Charon* mode defined with command-line arguments.
- default mode - the config is an array of *Matcher* objects. See an [example config file](/docs/example_config_default.json).
- immediate mode - the config is an array of *Action* objects. See an [example config file](/docs/example_config_immediate.json).
