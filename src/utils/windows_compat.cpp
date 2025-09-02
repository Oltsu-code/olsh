#include "../../include/utils/windows_compat.h"
#include <string>
#include <filesystem>
#include <algorithm>

namespace olsh::Utils {

std::string WindowsCompat::normalizePath(const std::string& path) {
    std::string normalized = path;

#ifdef _WIN32
    // replace the / with \ cz windows is ass
    std::replace(normalized.begin(), normalized.end(), '/', '\\');
#else
    std::replace(normalized.begin(), normalized.end(), '\\', '/');
#endif

    return normalized;
}

bool WindowsCompat::isAbsolutePath(const std::string& path) {
#ifdef _WIN32
    // again, no one likes windows paths
    return (path.length() >= 3 && path[1] == ':' &&
            (path[2] == '\\' || path[2] == '/')) ||
           (path.length() >= 2 && path[0] == '\\' && path[1] == '\\');
#else
    // Unix-like absolute paths start with /
    return !path.empty() && path[0] == '/';
#endif
}

std::string WindowsCompat::getExecutableExtension() {
#ifdef _WIN32
    return ".exe";
#else
    return "";
#endif
}

std::string WindowsCompat::findExecutable(const std::string& command) {
    if (command.find('.') != std::string::npos || isAbsolutePath(command)) {
        return command;
    }

    std::string withExt = command + getExecutableExtension();
    if (std::filesystem::exists(withExt)) {
        return withExt;
    }

    // search in PATH
    const char* pathEnv = getenv("PATH");
    if (pathEnv) {
        std::string pathStr(pathEnv);
        size_t start = 0;
        size_t end = 0;

#ifdef _WIN32
        char pathSep = ';';
#else
        char pathSep = ':';
#endif

        while ((end = pathStr.find(pathSep, start)) != std::string::npos) {
            std::string dir = pathStr.substr(start, end - start);
            std::string fullPath = dir + PATH_SEPARATOR_STR + command + getExecutableExtension();

            if (std::filesystem::exists(fullPath)) {
                return fullPath;
            }

            start = end + 1;
        }

        if (start < pathStr.length()) {
            std::string dir = pathStr.substr(start);
            std::string fullPath = dir + PATH_SEPARATOR_STR + command + getExecutableExtension();

            if (std::filesystem::exists(fullPath)) {
                return fullPath;
            }
        }
    }

    return command;
}

}