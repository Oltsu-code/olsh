# Builtins 

Like every other shell, even this one has builtins.


## List of builtins and usages
| Command                                          | Description                                                                                                                                                                                                               |
|--------------------------------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `cd <dir>`                                       | Change directory to `dir`. If `dir` is not provided, change to home                                                                                                                                                       |
| `exit`                                           | Exit the shell                                                                                                                                                                                                            |
| `cat <path>`                                     | Show the contents of the file in `path`                                                                                                                                                                                   |
| `history [-c]`                                   | Show or clear command history with `-c`                                                                                                                                                                                   |
| `alias [-d <name>] [<name> = <value>] [name]`    | List all aliases, delete with `-d <name>` (`name` being the name of the alias), add with `<name> = <value>` (`name` is the name of the alias and `value` is the value its set to) or look up the value of one with `name` |      
| `clear`                                          | Clear the terminal                                                                                                                                                                                                        |
| `ls [-l] [-a] [<dir>]`                           | List contents of the current or another dir with `<dir>`. To list hidden file use `-a`. To get more info, use `-l`                                                                                                        |
| `pwd`                                            | Print the current working directory                                                                                                                                                                                       |
| `echo [-n] <text>`                               | Print text to the console. You can also use `-n` to skip creating a new line                                                                                                                                              |
| `rm [-r] [-f] <path (or dir)>`                   | Remove a path or dir with `-r`. To skip all protections, use `-f` to force delete                                                                                                                                         |
| `config [-s] [-g <key>] [-S <key> <value>] [-h]` | Show current config with `-s`. Get a specific value from the config with `-g <key>`. Set a value with `-S <key> <value>`                                                                                                  |

## Notes
- All the flags can be combined, e.g. `ls -la` or `rm -rf`
- `cd` supports `..` and `.` for parent and current directory respectively
- You can also use `~` to refer to your home directory (e.g. `cd ~/Documents` or `cat ~/file.txt`)
- Most off the builtins also have long flags (e.g. `config --set`)
- Builtins can also be skipped with `^` (e.g. `^cd`)