#ifndef WINDOWS_COMPAT_H
#define WINDOWS_COMPAT_H

#include <string>

#ifdef _WIN32
    #include <windows.h>
    #include <io.h>
    #include <direct.h>
    #define PATH_SEPARATOR '\\'
    #define PATH_SEPARATOR_STR "\\"
    #define HOME_ENV "USERPROFILE"
#else
    #include <unistd.h>
    #include <sys/stat.h>
    #define PATH_SEPARATOR '/'
    #define PATH_SEPARATOR_STR "/"
    #define HOME_ENV "HOME"
#endif

namespace olsh::Utils {

class WindowsCompat {
public:
    static std::string normalizePath(const std::string& path);
    static bool isAbsolutePath(const std::string& path);
    static std::string getExecutableExtension();
    static std::string findExecutable(const std::string& command);
};

} // namespace olsh::Utils

#endif //WINDOWS_COMPAT_H
