#include "../../include/builtins/weather.h"
#include "../../include/utils/colors.h"
#include <iostream>
#include <filesystem>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

namespace olsh::Builtins {

int Weather::execute(const std::vector<std::string>& args) {
    // Check if Ruby is installed
    if (!checkRubyInstalled()) {
        std::cout << RED << "❌ Error: Ruby is not installed or not in PATH" << RESET << std::endl;
        std::cout << YELLOW << "💡 Please install Ruby to use the weather command" << RESET << std::endl;
        std::cout << CYAN << "   Visit: https://www.ruby-lang.org/en/downloads/" << RESET << std::endl;
        return 1;
    }

    std::string scriptPath = getScriptPath();
    
    // Check if script exists
    if (!std::filesystem::exists(scriptPath)) {
        std::cout << RED << "❌ Error: Weather script not found at: " << scriptPath << RESET << std::endl;
        return 1;
    }

    // Build command
    std::string command = "ruby \"" + scriptPath + "\"";
    
    // Add city argument if provided
    if (args.size() > 1) {
        command += " \"" + args[1] + "\"";
    }

    // Execute the Ruby script
#ifdef _WIN32
    int result = system(command.c_str());
#else
    int result = system(command.c_str());
#endif

    return result;
}

std::string Weather::getScriptPath() {
    // Get the directory of the current executable
    std::string execPath;
    
#ifdef _WIN32
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    execPath = buffer;
#else
    char buffer[1024];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len != -1) {
        buffer[len] = '\0';
        execPath = buffer;
    }
#endif

    std::filesystem::path path(execPath);
    std::filesystem::path scriptDir = path.parent_path().parent_path() / "scripts";
    return (scriptDir / "weather.rb").string();
}

bool Weather::checkRubyInstalled() {
#ifdef _WIN32
    // Try to run ruby --version and check if it succeeds
    int result = system("ruby --version >nul 2>&1");
    return result == 0;
#else
    int result = system("ruby --version > /dev/null 2>&1");
    return WEXITSTATUS(result) == 0;
#endif
}

}
