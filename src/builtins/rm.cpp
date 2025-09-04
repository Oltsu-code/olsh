#include "../../include/builtins/rm.h"
#include <utils/colors.h>
#include <iostream>
#include <filesystem>

namespace olsh::Builtins {

int Rm::execute(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << RED << "rm: missing operand\n" << RESET;
        std::cerr << "Usage: rm [-rf] [--] <file/directory>...\n";
        return 1;
    }

    bool recursive = false;
    bool force = false;

    // parse args (ls-style): support -rf, --force, --recursive, and -- end of options
    std::vector<std::string> targets;
    bool endOfOptions = false;
    for (const auto& arg : args) {
        if (!endOfOptions && !arg.empty() && arg[0] == '-' && arg.size() > 1) {
            if (arg == "--") { endOfOptions = true; continue; }
            if (arg.rfind("--", 0) == 0) {
                if (arg == "--force") force = true;
                else if (arg == "--recursive") recursive = true;
                else {
                    std::cerr << RED << "rm: unrecognized option '" << arg << "'\n" << RESET;
                    std::cerr << "Usage: rm [-rf] [--] <file/directory>...\n";
                    return 1;
                }
                continue;
            }
            // short options bundle
            for (size_t j = 1; j < arg.size(); ++j) {
                switch (arg[j]) {
                    case 'r': case 'R': recursive = true; break;
                    case 'f': force = true; break;
                    default:
                        std::cerr << RED << "rm: invalid option -- '" << arg[j] << "'\n" << RESET;
                        std::cerr << "Usage: rm [-rf] [--] <file/directory>...\n";
                        return 1;
                }
            }
        } else {
            // positional
            targets.push_back(arg);
        }
    }

    if (targets.empty()) {
        std::cerr << RED << "rm: missing operand\n" << RESET;
        std::cerr << "Usage: rm [-rf] [--] <file/directory>...\n";
        return 1;
    }

    int result = 0;
    for (const auto& target : targets) {
        try {
            const std::filesystem::path p(target);
            if (!std::filesystem::exists(p)) {
                if (!force) {
                    std::cerr << RED << "rm: cannot remove '" << target << "': No such file or directory\n" << RESET;
                    result = 1;
                }
                continue; // skip missing files if -f
            }

            if (std::filesystem::is_directory(p)) {
                if (recursive) {
                    std::filesystem::remove_all(p);
                } else if (!force) {
                    std::cerr << RED << "rm: cannot remove '" << target
                              << "': Is a directory (use -r for recursive removal)\n"
                              << RESET;
                    result = 1;
                }
            } else {
                std::filesystem::remove(p);
            }
        } catch (const std::filesystem::filesystem_error& e) {
            if (!force) {
                std::cerr << RED << "rm: cannot remove '" << target << "': " << e.what() << RESET << std::endl;
                result = 1;
            }
            // if force, ignore exceptions
        }
    }

    return result;
}

} // namespace olsh::Builtins
