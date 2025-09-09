#include "utils/input_manager.h"
#include "shell.h"
#include <iostream>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#endif

namespace olsh::Utils {

    std::unique_ptr<InputManager> InputManager::instance = nullptr;
olsh::Shell* InputManager::shell_instance = nullptr;

InputManager::InputManager() {
    readlineSetCompletionCallback(completionCallback);
    readlineHistorySetMaxLen(1000); // should be plenty for most users
    readlineSetMultiLine(0);
}

InputManager& InputManager::getInstance() {
    if (!instance) {
        instance = std::make_unique<InputManager>();
    }
    return *instance;
}

void InputManager::setShellInstance(olsh::Shell* shell) {
    shell_instance = shell;
}

std::string InputManager::readLine(const std::string& prompt) {
    bool isInteractive = true;
    
#ifdef _WIN32
    DWORD mode;
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    if (!GetConsoleMode(hInput, &mode)) {
        isInteractive = false; // not a console
    }
#else
    if (!isatty(STDIN_FILENO)) {
        isInteractive = false;
    }
#endif

    if (!isInteractive) {
        std::cout << prompt << std::flush;
        std::string line;
        if (std::getline(std::cin, line)) {
            return line;
        } else {
            return "\x04";
        }
    } else {
        char* line = readline(prompt.c_str());
        
        if (!line) {
            // null return indicates EOF (Ctrl+D) - return special marker
            return "\x04"; // ASCII EOT (End of Transmission)
        }
        
        std::string result(line);
        readlineFree(line); // clean memory
        
        return result;
    }
}

void InputManager::addToHistory(const std::string& line) {
    if (!line.empty()) {
        readlineHistoryAdd(line.c_str());
    }
}

bool InputManager::saveHistory(const std::string& filename) {
    if (filename.empty()) return false;
    
    // Ensure directory exists
    std::filesystem::path histPath(filename);
    std::filesystem::create_directories(histPath.parent_path());
    
    return readlineHistorySave(filename.c_str()) == 0;
}

bool InputManager::loadHistory(const std::string& filename) {
    if (filename.empty()) return false;
    
    return readlineHistoryLoad(filename.c_str()) == 0;
}

void InputManager::completionCallback(const char* input, readlineCompletions* completions) {
    if (!shell_instance) {
        return; 
    }
    
    std::string inputStr(input);
    auto suggestions = shell_instance->autocomplete(inputStr, inputStr.length());

    // add each suggestion to readline
    for (const auto& suggestion : suggestions) {
        readlineAddCompletion(completions, suggestion.c_str());
    }
}

InputManager::~InputManager() {
    
}

} // namespace olsh::Utils
