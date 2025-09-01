cmake_minimum_required(VERSION 3.31)
project(olshell)

set(CMAKE_CXX_STANDARD 26)

# Tell CMake where to find headers
include_directories(include)

# Only list .cpp files here
add_executable(olshell
        src/main.cpp
        src/shell.cpp
        src/parser/parser.cpp
        src/parser/tokenizer.cpp
        src/parser/ast.cpp
        src/executor/executor.cpp
        src/executor/process.cpp
        src/builtins/cd.cpp
        src/builtins/ls.cpp
        src/builtins/rm.cpp
        src/builtins/echo.cpp
        src/builtins/pwd.cpp
        src/builtins/cat.cpp
        src/builtins/clear.cpp
        src/builtins/history.cpp
        src/builtins/alias.cpp
        src/builtins/config.cpp
        src/utils/fs.cpp
        src/utils/string.cpp
        src/utils/windows_compat.cpp
        src/utils/autocomplete.cpp
        src/utils/script.cpp
        src/utils/config.cpp
        src/utils/linenoise.cpp
        src/utils/input_manager.cpp
        src/builtins/builtin_registry.cpp
)
