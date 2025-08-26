#include "../../include/builtins/echo.h"
#include <iostream>

namespace olsh::Builtins {

int Echo::execute(const std::vector<std::string>& args) {
    bool newline = true;
    size_t start = 0;

    // parse flag
    if (!args.empty() && args[0] == "-n") {
        newline = false;
        start = 1;
    }

    // print the args
    for (size_t i = start; i < args.size(); ++i) {
        if (i > start) std::cout << " ";
        std::cout << args[i];
    }

    if (newline) std::cout << std::endl;

    return 0;
}

}