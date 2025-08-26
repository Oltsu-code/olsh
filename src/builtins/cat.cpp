#include "../../include/builtins/cat.h"
#include <utils/colors.h>
#include <iostream>
#include <fstream>
#include <vector>

namespace olsh::Builtins {

int Cat::execute(const std::vector<std::string>& args) {
    if (args.empty()) {
        // read from stdin if no args yk
        std::string line;
        while (std::getline(std::cin, line)) {
            std::cout << line << std::endl;
        }
        return 0;
    }

    for (const auto& filename : args) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << RED << "cat: " << filename << ": No such file or directory" << RESET << std::endl;
            return 1;
        }

        std::string line;
        while (std::getline(file, line)) {
            std::cout << line << std::endl;
        }
        file.close();
    }

    return 0;
}

}
