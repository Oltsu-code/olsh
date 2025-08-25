#include "../../include/builtins/ls.h"
#include <iostream>
#include <filesystem>
#include <iomanip>

namespace olsh::Builtins {

int Ls::execute(const std::vector<std::string>& args) {
    std::string path = ".";
    bool showHidden = false;
    bool longFormat = false;

    // parse args
    for (const auto& arg : args) {
        if (arg == "-a" || arg == "--all") {
            showHidden = true;
        } else if (arg == "-l" || arg == "--long") {
            longFormat = true;
        } else if (arg[0] != '-') {
            path = arg;
        }
    }

    try {
        if (std::filesystem::is_directory(path)) {
            for (const auto& entry : std::filesystem::directory_iterator(path)) {
                std::string filename = entry.path().filename().string();

                // Skip hidden files unless -a is specified
                if (!showHidden && filename[0] == '.') {
                    continue;
                }

                if (longFormat) {
                    // Show detailed information
                    auto size = entry.is_regular_file() ? std::filesystem::file_size(entry) : 0;
                    std::cout << (entry.is_directory() ? "d" : "-");
                    std::cout << "rwxrwxrwx ";  // Simplified permissions
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