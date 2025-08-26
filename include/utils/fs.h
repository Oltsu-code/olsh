#ifndef FS_H
#define FS_H

#include <string>
#include <vector>

namespace olsh::Utils {

class Fs {
public:
    static bool exists(const std::string& path);
    static bool isFile(const std::string& path);
    static bool isDirectory(const std::string& path);
    static std::string normalizePath(const std::string& path);
    static std::string expandPath(const std::string& path);
    static std::string getHomeDirectory();
    static std::string getCurrentDirectory();
    static std::vector<std::string> listDirectory(const std::string& path);
    static bool createDirectory(const std::string& path);
    static bool removeFile(const std::string& path);
    static bool removeDirectory(const std::string& path, bool recursive = false);
};

}

#endif //FS_H
