#include "../../include/builtins/help.h"
#include <iostream>

namespace olsh::Builtins {

int Help::execute(const std::vector<std::string>& args) {
    std::cout << "OLShell v1.0 - Available Commands:\n";
    std::cout << "=====================================\n\n";

    std::cout << "Built-in Commands:\n";
    std::cout << "  cd [directory]     - Change directory \n";
    std::cout << "  ls [-a] [-l] [dir] - List directory contents\n";
    std::cout << "                       -a: show hidden files\n";
    std::cout << "                       -l: long format\n";
    std::cout << "  pwd                - Print current directory\n";
    std::cout << "  echo [-n] [text]   - Display text\n";
    std::cout << "                       -n: no trailing newline\n";
    std::cout << "  rm [-r] <files>    - Remove files/directories\n";
    std::cout << "                       -r: recursive removal\n";
    std::cout << "  cat <files>        - Concatenate and display file contents\n";
    std::cout << "  clear              - Clear the terminal screen\n";
    std::cout << "  history            - Show command history\n";
    std::cout << "  joke               - Tell a random programming joke\n";
    std::cout << "  alias [name[=value] ...] - Define or display aliases\n";
    std::cout << "  help               - Show this help message\n";
    std::cout << "  exit               - Exit the shell\n\n";

    std::cout << "Features:\n";
    std::cout << "  - Command piping with '|'\n";
    std::cout << "  - Output redirection with '>' and '>>'\n";
    std::cout << "  - Input redirection with '<'\n";
    std::cout << "  - External command execution\n";
    std::cout << "  - Quoted string support\n\n";

    std::cout << "Examples:\n";
    std::cout << "  ls -la\n";
    std::cout << "  echo \"Hello World\" > output.txt\n";
    std::cout << "  ls | grep txt\n";
    std::cout << "  cd /path/to/directory\n\n";

    return 0;
}

}