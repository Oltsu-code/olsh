#include "../../include/builtins/cd.h"
#include "../../include/utils/fs.h"
#include <utils/colors.h>
#include <iostream>
#include <filesystem>

namespace olsh::Builtins {

int Cd::execute(const std::vector<std::string>& args) {
    std::string path;

    if (args.empty()) {
        // no args means go home
        path = olsh::Utils::Fs::getHomeDirectory();
    } else {
        path = olsh::Utils::Fs::expandPath(args[0]);
    }

    try {
        std::filesystem::current_path(path);
        return 0;
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << RED << "cd: " << e.what() << RESET << std::endl;
        return 1;
    }
}

} // namespace olsh::Builtins
