#include "../../include/builtins/mv.h"

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <utils/colors.h>

namespace olsh::Builtins {

int Mv::execute(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << RED << "mv: missing operand\n" << RESET;
        std::cerr << "Usage: mv [-finuv] <source> <destination>\n";
        return 1;
    }

    bool force = false;
    bool interactive = false;
    bool noClobber = false;
    bool verbose = false;
    bool update = false;

    std::string src;
    std::string dest;

    // parse flags
    for (const auto& arg : args) {
        if (!arg.empty() && arg[0] == '-' && arg.size() > 1) {
            for (size_t i = 1; i < arg.size(); ++i) {
                switch (arg[i]) {
                    case 'f': force = true; break;
                    case 'i': interactive = true; break;
                    case 'n': noClobber = true; break;
                    case 'v': verbose = true; break;
                    case 'u': update = true; break;
                    default:
                        std::cerr << RED << "mv: invalid option: -" << arg[i] << RESET << "\n";
                        std::cerr << "Usage: mv [-finuv] <source> <destination>\n";
                        return 1;
                }
            }
        } else {
            // source and destination
            if (src.empty()) {
                src = arg;
            } else if (dest.empty()) {
                dest = arg;
            } else {
                std::cerr << RED << "mv: too many operands\n" << RESET;
                std::cerr << "Usage: mv [-finuv] <source> <destination>\n";
                return 1;
            }
        }
    }

    if (src.empty() || dest.empty()) {
        std::cerr << RED << "mv: missing file operand\n" << RESET;
        std::cerr << "Usage: mv [-finuv] <source> <destination>\n";
        return 1;
    }

    if (noClobber && std::filesystem::exists(dest)) {
        if (verbose) {
            std::cout << "Not overwriting '" << dest << "' (no-clobber).\n";
        }
        return 0;
    }

    try {
        if (!force && interactive && std::filesystem::exists(dest)) {
            std::cout << "mv: overwrite '" << dest << "'? (y/n) ";
            char response{};
            std::cin >> response;
            if (response != 'y' && response != 'Y') {
                if (verbose) {
                    std::cout << "Not overwriting '" << dest << "'.\n";
                }
                return 0;
            }
        }

        if (update && std::filesystem::exists(dest)) {
            auto srcTime  = std::filesystem::last_write_time(src);
            auto destTime = std::filesystem::last_write_time(dest);
            if (srcTime <= destTime) {
                if (verbose) {
                    std::cout << "Not moving '" << src
                              << "' to '" << dest
                              << "' because destination is newer.\n";
                }
                return 0;
            }
        }

        std::error_code ec;
        std::filesystem::rename(src, dest, ec);
        if (ec) {
            if (ec.value() == EXDEV) {
                std::filesystem::copy(src, dest,
                    std::filesystem::copy_options::overwrite_existing |
                    std::filesystem::copy_options::recursive,
                    ec);
                if (ec) {
                    std::cerr << RED << "mv: error copying file: " << ec.message() << RESET << "\n";
                    return 1;
                }
                std::filesystem::remove_all(src, ec);
                if (ec) {
                    std::cerr << RED << "mv: error removing source: " << ec.message() << RESET << "\n";
                    return 1;
                }
            } else {
                std::cerr << RED << "mv: error moving file: " << ec.message() << RESET << "\n";
                return 1;
            }
        }

        if (verbose) {
            std::cout << "Moved '" << src << "' to '" << dest << "'.\n";
        }

    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << RED << "mv: error: " << e.what() << RESET << "\n";
        return 1;
    }

    return 0;
}

} // namespace olsh::Builtins


