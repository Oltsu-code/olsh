# Scripting in olshell

Even this hunk of junk has scripting capabilities. They are very limited, but they exist.

## Variables    
You can set variables with the `set` builtin:

```
set VAR_NAME = value
```

You can then use the variable with `$VAR_NAME`:

```
echo $VAR_NAME
```
You can also use variables in commands, e.g. `cat $FILE_PATH`.
You can also use command substitution with backticks:

```
set FILES = `ls`
echo $FILES
```
This will set `FILES` to the output of `ls`.
You can also use arithmetic expansion with `$((expression))`:

```
    
set NUM = $((1 + 2))
echo $NUM
```
This will set `NUM` to `3`.
You can also use environment variables, e.g. `$HOME`, `$PATH`, etc.
    
## Conditionals
You can use `if`, `elif`, `else`, and `fi` for conditionals
```
if [ condition ]; then
    # commands
elif [ condition ]; then
    # commands
else
    # commands
fi
```

Conditions can be comparisons like `-eq`, `-ne`, `-lt`, `-le`, `-gt`, `-ge`, `-z`, `-n`, etc.
Example:
```
set NUM = 5

        
if [ $NUM -gt 0 ]; then
    echo "NUM is positive"
elif [ $NUM -lt 0 ]; then
    echo "NUM is negative"
else
    echo "NUM is zero"
fi
```
## Loops
You can use `for`, `while`, and `do` for loops:
```
for VAR in list; do
    # commands
done
while [ condition ]; do
    # commands
done
```
Example:
```
for FILE in `ls`; do
    echo $FILE
done
```
```
set COUNT = 0
while [ $COUNT -lt 5 ]; do
    echo $COUNT
    set COUNT = $((COUNT + 1))
done
```
## Functions
You can define functions with the `function` keyword:
```
function func_name {
    # commands
}
```
You can then call the function with `func_name`.
Example:
```
function greet {
    echo "Hello, $1!"
}
greet "World"
```
## Scripts
You can write scripts in a file and execute them with `./script.olsh` or `olsh
    script.olsh`.
Make sure to give execute permission to the script with `chmod +x script.olsh`.
Example script (`script.olsh`):
```
#!/bin/olshell
set NAME = $1
echo "Hello, $NAME!"
```
Run the script:
```
./script.olsh "World"
```
This will output `Hello, World!`.
## Notes
- Scripts can use all the features mentioned above.
- Scripts can also use builtins and external commands.
- Scripts can take arguments, which can be accessed with `$1`, `$2`, etc.
- Scripts can also use `$@` to refer to all arguments and `$#` to refer
- to the number of arguments.
- Make sure to handle errors and edge cases in your scripts.
- This shell is very limited, so some features may not work as expected.
- Have fun scripting!
- Remember, this is the world's worst shell, so don't expect too much!
- Happy coding!
- :P
- :)
- :D
- ^_^
- <3
- ~_~
- :/
- 