#include "../../include/utils/fs.h"
#include <utils/colors.h>
#include <filesystem>
#include <iostream>

namespace olsh::Utils {

std::string Fs::expandPath(const std::string& path) {
    if (path.empty() || path[0] != '~') {
        return path;
    }

    std::string homeDir = getHomeDirectory();
    if (path == "~") {
        return homeDir;
    } else if (path.size() > 1 && (path[1] == '/' || path[1] == '\\')) {
        return homeDir + path.substr(1);
    }

    return path;
}

std::string Fs::normalizePath(const std::string& path) {
    std::string homeDir = getHomeDirectory();
    std::filesystem::path fsPath = std::filesystem::absolute(path);
    std::string absolutePath = fsPath.string();

    // Replace home directory with ~
    if (absolutePath.find(homeDir) == 0) {
        if (absolutePath == homeDir) {
            return "~";
        } else if (absolutePath.size() > homeDir.size() &&
                   (absolutePath[homeDir.size()] == '/' || absolutePath[homeDir.size()] == '\\')) {
            return "~" + absolutePath.substr(homeDir.size());
        }
    }

    return absolutePath;
}

bool Fs::exists(const std::string& path) {
    return std::filesystem::exists(expandPath(path));
}

bool Fs::isFile(const std::string& path) {
    return std::filesystem::is_regular_file(expandPath(path));
}

bool Fs::isDirectory(const std::string& path) {
    return std::filesystem::is_directory(expandPath(path));
}

std::string Fs::getHomeDirectory() {
    const char* home = getenv("USERPROFILE");  // windows
    if (!home) {
        home = getenv("HOME");  // linux and shit
    }
    return home ? std::string(home) : ".";
}

std::string Fs::getCurrentDirectory() {
    return normalizePath(std::filesystem::current_path().string());
}

std::vector<std::string> Fs::listDirectory(const std::string& path) {
    std::vector<std::string> files;
    try {
        for (const auto& entry : std::filesystem::directory_iterator(expandPath(path))) {
            files.push_back(entry.path().filename().string());
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << RED << "Error listing directory: " << e.what() << RESET << std::endl;
    }
    return files;
}

bool Fs::createDirectory(const std::string& path) {
    try {
        return std::filesystem::create_directories(expandPath(path));
    } catch (const std::filesystem::filesystem_error&) {
        return false;
    }
}

bool Fs::removeFile(const std::string& path) {
    try {
        return std::filesystem::remove(expandPath(path));
    } catch (const std::filesystem::filesystem_error&) {
        return false;
    }
}

bool Fs::removeDirectory(const std::string& path, bool recursive) {
    try {
        std::string expandedPath = expandPath(path);
        if (recursive) {
            return std::filesystem::remove_all(expandedPath) > 0;
        } else {
            return std::filesystem::remove(expandedPath);
        }
    } catch (const std::filesystem::filesystem_error&) {
        return false;
    }
}

} // namespace olsh::Utils