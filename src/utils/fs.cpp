#include "../../include/utils/fs.h"
#include <filesystem>
#include <iostream>

namespace olsh::Utils {

bool Fs::exists(const std::string& path) {
    return std::filesystem::exists(path);
}

bool Fs::isFile(const std::string& path) {
    return std::filesystem::is_regular_file(path);
}

bool Fs::isDirectory(const std::string& path) {
    return std::filesystem::is_directory(path);
}

std::string Fs::getHomeDirectory() {
    const char* home = getenv("USERPROFILE");  // windows
    if (!home) {
        home = getenv("HOME");  // linux and shit
    }
    return home ? std::string(home) : ".";
}

std::string Fs::getCurrentDirectory() {
    return std::filesystem::current_path().string();
}

std::vector<std::string> Fs::listDirectory(const std::string& path) {
    std::vector<std::string> files;
    try {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            files.push_back(entry.path().filename().string());
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error listing directory: " << e.what() << std::endl;
    }
    return files;
}

bool Fs::createDirectory(const std::string& path) {
    try {
        return std::filesystem::create_directories(path);
    } catch (const std::filesystem::filesystem_error&) {
        return false;
    }
}

bool Fs::removeFile(const std::string& path) {
    try {
        return std::filesystem::remove(path);
    } catch (const std::filesystem::filesystem_error&) {
        return false;
    }
}

bool Fs::removeDirectory(const std::string& path, bool recursive) {
    try {
        if (recursive) {
            return std::filesystem::remove_all(path) > 0;
        } else {
            return std::filesystem::remove(path);
        }
    } catch (const std::filesystem::filesystem_error&) {
        return false;
    }
}

}