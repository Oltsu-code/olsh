#include "../../include/builtins/rm.h"
#include <utils/colors.h>
#include <iostream>
#include <filesystem>

namespace olsh::Builtins {

int Rm::execute(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << RED << "rm: missing operand\n" << RESET;
        std::cerr << "Usage: rm [-r] <file/directory>...\n";
        return 1;
    }

    bool recursive = false;
    size_t start = 0;

    // -r flag
    if (!args.empty() && args[0] == "-r") {
        recursive = true;
        start = 1;
    }

    // TODO: add -f flag so we can delete our roots for fun

    if (start >= args.size()) {
        std::cerr << RED << "rm: missing operand\n" << RESET;
        return 1;
    }

    int result = 0;
    for (size_t i = start; i < args.size(); ++i) {
        try {
            if (std::filesystem::is_directory(args[i])) {
                if (recursive) {
                    std::filesystem::remove_all(args[i]);
                } else {
                    std::cerr << RED << "rm: cannot remove '" << args[i]
                                     << "': Is a directory (use -r for recursive removal)\n"
                                     << RESET;
                    result = 1;
                }
            } else {
                std::filesystem::remove(args[i]);
            }
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << RED << "rm: cannot remove '" << args[i] << "': " << e.what() << RESET << std::endl;
            result = 1;
        }
    }

    return result;
}

}
