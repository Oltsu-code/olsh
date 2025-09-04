#include "../../include/builtins/clear.h"
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>

namespace olsh::Builtins {

int Clear::execute(const std::vector<std::string>& args) {
    bool clearScrollback = false;

    for (const auto& a : args) {
        if (!a.empty() && a[0] == '-') {
            if (a == "-x" || a == "--scrollback") clearScrollback = true;
            else if (a == "--") break; 
            else {
                std::cerr << "clear: unknown option '" << a << "'\n";
                std::cerr << "Usage: clear [-x|--scrollback]" << std::endl;
                return 1;
            }
        }
    }

#ifdef _WIN32
    if (clearScrollback) {
        // clear screen and scrollback buffer
        std::system("cls && echo. > nul");
    } else {
        std::system("cls");
    }
#else
    if (clearScrollback) {
        // clear screen and scrollback buffer on unix
        std::system("clear && printf '\\e[3J'");
    } else {
        std::system("clear");
    }
#endif

    return 0;
}

} // namespace olsh::Builtins