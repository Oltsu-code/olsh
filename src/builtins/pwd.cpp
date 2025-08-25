#include "../../include/builtins/pwd.h"
#include <iostream>
#include <filesystem>

namespace olsh::Builtins {

int Pwd::execute(const std::vector<std::string>& args) {
    try {
        std::cout << std::filesystem::current_path().string() << std::endl;
        return 0;
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "pwd: " << e.what() << std::endl;
        return 1;
    }
}

}