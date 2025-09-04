#include "../../include/builtins/mkdir.h"
#include "../../include/utils/fs.h"
#include <utils/colors.h>
#include <iostream>
#include <filesystem>

namespace olsh::Builtins {

int Mkdir::execute(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << RED << "mkdir: missing operand" << RESET << std::endl;
        return 1;
    }

    for (const auto& dir : args) {
        std::string path = olsh::Utils::Fs::expandPath(dir);
        try {
            if (!std::filesystem::create_directories(path)) {
                std::cerr << RED << "mkdir: cannot create directory '" << dir
                          << "': Directory already exists" << RESET << std::endl;
                return 1;
            }
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << RED << "mkdir: cannot create directory '" << dir
                      << "': " << e.what() << RESET << std::endl;
            return 1;
        }
    }
    return 0;

}

}