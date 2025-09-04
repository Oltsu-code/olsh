#include "../../include/builtins/cp.h"
#include <utils/colors.h>
#include <filesystem>
#include <iostream>

namespace olsh::Builtins {

int Cp::execute(const std::vector<std::string> &args) {
    if (args.size() < 3) {
        std::cerr << RED << "cp: missing file operand\n" << RESET;
        std::cerr << "Usage: cp <source> <destination>\n";
        return 1;
    }

    std::filesystem::path source = args[1];
    std::filesystem::path destination = args[2];

    if (std::filesystem::exists(destination) && std::filesystem::is_directory(destination)) {
        destination /= source.filename();
    }

    try {
        std::filesystem::copy(source, destination,
            std::filesystem::copy_options::overwrite_existing |
            std::filesystem::copy_options::recursive);
    } catch (const std::filesystem::filesystem_error &e) {
        std::cerr << RED << "cp: " << e.what() << "\n" << RESET;
        return 1;
    }

    return 0;
}

} // namespace olsh::Builtins
