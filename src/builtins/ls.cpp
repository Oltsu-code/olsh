#include "../../include/builtins/ls.h"
#include <iostream>
#include <filesystem>
#include <iomanip>

#include "utils/colors.h"

namespace olsh::Builtins {

int Ls::execute(const std::vector<std::string>& args) {
    std::string path = ".";
    bool showHidden = false;
    bool longFormat = false;

    // parse args
    for (const auto& arg : args) {
        if (arg[0] == '-' && arg.size() > 1) {
            for (size_t i = 1; i < arg.size(); i++) {
                switch(arg[i]) {
                    case 'a': showHidden = true; break;
                    case 'l': longFormat = true; break;
                    default:
                        std::cerr << RED << "Unknown option: -" << arg[i] << RESET << "\n";
                }
            }
        } else if (arg == "--all") {
            showHidden = true;
        } else if (arg == "--long") {
            longFormat = true;
        } else {
            path = arg; // positional argument
        }
    }


    try {
        if (std::filesystem::is_directory(path)) {
            for (const auto& entry : std::filesystem::directory_iterator(path)) {
                std::string filename = entry.path().filename().string();

                // skip hidden files
                if (!showHidden && filename[0] == '.') {
                    continue;
                }

                if (longFormat) {
                    auto size = entry.is_regular_file() ? std::filesystem::file_size(entry) : 0;
                    std::cout << (entry.is_directory() ? "d" : "-");
                    std::cout << "rwxrwxrwx "; // permissions (not accurate, just a placeholder)
                    std::cout << std::setw(8) << size << " ";
                    std::cout << filename << std::endl;
                } else {
                    std::cout << filename;
                    if (entry.is_directory()) std::cout << "/";
                    std::cout << "  ";
                }
            }
            if (!longFormat) std::cout << std::endl;
        } else {
            std::cout << path << std::endl;
        }
        return 0;
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "ls: " << e.what() << std::endl;
        return 1;
    }
}

}
