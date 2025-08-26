#include "../../include/builtins/pwd.h"
#include <utils/colors.h>
#include <iostream>
#include <filesystem>

namespace olsh::Builtins {

int Pwd::execute(const std::vector<std::string>& args) {
    try {
        std::cout << std::filesystem::current_path().string() << std::endl;
        return 0;
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << RED << "pwd: " << e.what() << RESET << std::endl;
        return 1;
    }
}

}