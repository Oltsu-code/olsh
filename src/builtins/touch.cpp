#include "../../include/builtins/touch.h"
#include <utils/colors.h>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <chrono>

namespace olsh::Builtins {

int Touch::execute(const std::vector<std::string> &args) {
    if (args.empty()) {
        std::cerr << RED << "touch: missing file operand\n" << RESET;
        std::cerr << "Usage: touch [options] <file>...\n";
        return 1;
    }

    bool changeAccess = true;
    bool changeModify = true;
    bool noCreate = false;
    std::filesystem::file_time_type refTime;
    bool useReference = false;

    std::vector<std::string> files;

    // parse flags
    for (size_t i = 0; i < args.size(); ++i) {
        const auto &arg = args[i];
        if (arg == "-a") {
            changeModify = false;
        } else if (arg == "-m") {
            changeAccess = false;
        } else if (arg == "-c") {
            noCreate = true;
        } else if (arg == "-r") {
            if (i + 1 >= args.size()) {
                std::cerr << RED << "touch: option requires an argument -- 'r'\n" << RESET;
                return 1;
            }
            std::filesystem::path refPath = args[++i];
            if (!std::filesystem::exists(refPath)) {
                std::cerr << RED << "touch: cannot stat '" << refPath << "': No such file\n" << RESET;
                return 1;
            }
            refTime = std::filesystem::last_write_time(refPath);
            useReference = true;
        } else {
            files.push_back(arg);
        }
    }

    if (files.empty()) {
        std::cerr << RED << "touch: missing file operand\n" << RESET;
        return 1;
    }

    try {
        for (const auto &file : files) {
            std::filesystem::path p(file);

            if (!std::filesystem::exists(p)) {
                if (noCreate) continue;

                // create file
                std::ofstream ofs(p);
                if (!ofs) {
                    std::cerr << RED << "touch: cannot create file '" << p << "'\n" << RESET;
                    return 1;
                }
            }

            auto now = std::chrono::file_clock::now();
            auto newTime = useReference ? refTime : now;

            // update times
            if (changeAccess && changeModify) {
                std::filesystem::last_write_time(p, newTime);
            } else {
                if (changeModify) {
                    std::filesystem::last_write_time(p, newTime);
                }
            }
        }
    } catch (const std::filesystem::filesystem_error &e) {
        std::cerr << RED << "touch: " << e.what() << "\n" << RESET;
        return 1;
    }

    return 0;
}

} // namespace olsh::Builtins
