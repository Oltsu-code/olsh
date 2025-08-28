# Builtins 

Like every other shell, even this one has builtins.


## List of builtins and usages
| Command                                       | Description                                                                                                                                                                                                               |
|-----------------------------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `cd <dir>`                                    | Change directory to `dir`. If `dir` is not provided, change to home                                                                                                                                                       |
| `exit`                                        | Exit the shell                                                                                                                                                                                                            |
| `cat <path>`                                  | Show the contents of the file in `path`                                                                                                                                                                                   |
| `history [-c]`                                | Show or clear command history with `-c`                                                                                                                                                                                   |
| `alias [-d <name>] [<name> = <value>] [name]` | List all aliases, delete with `-d <name>` (`name` being the name of the alias), add with `<name> = <value>` (`name` is the name of the alias and `value` is the value its set to) or look up the value of one with `name` |      
| `clear`                                       | Clear the terminal                                                                                                                                                                                                        |
| `ls [-l] [-a] [<dir>]`                        | List contents of the current or another dir with `<dir>`. To list hidden file use `-a`. To get more info, use `-l`                                                                                                        |
| `pwd`                                         | Print the current working directory                                                                                                                                                                                       |
| `echo [-n] <text>`                            | Print text to the console. You can also use `-n` to skip creating a new line                                                                                                                                              |
| `rm [-r] [-f] <path (or dir)>`                | Remove a path or dir with `-r`. To skip all protections, use `-f` to force delete                                                                                                                                         |
| `weather [city]`                              | Get current weather and forecast for a city. If no city is specified, uses default location. Requires Ruby to be installed.                                                                                              |

## Notes
- All the flags can be combined, e.g. `ls -la` or `rm -rf`
- `cd` supports `..` and `.` for parent and current directory respectively
- You can also use `~` to refer to your home directory (e.g. `cd ~/Documents` or `cat ~/file.txt`)