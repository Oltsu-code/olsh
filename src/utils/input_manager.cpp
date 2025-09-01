#include "utils/input_manager.h"
#include "shell.h"
#include <iostream>
#include <filesystem>

namespace olsh::Utils {

    std::unique_ptr<InputManager> InputManager::instance = nullptr;
olsh::Shell* InputManager::shell_instance = nullptr;

InputManager::InputManager() {
    linenoiseSetCompletionCallback(completionCallback);
    linenoiseHistorySetMaxLen(1000); //? prob too much
    linenoiseSetMultiLine(0);
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
    char* line = linenoise(prompt.c_str());
    
    if (!line) {
        return "";
    }
    
    std::string result(line);
    linenoiseFree(line); // clean memory
    
    return result;
}

void InputManager::addToHistory(const std::string& line) {
    if (!line.empty()) {
        linenoiseHistoryAdd(line.c_str());
    }
}

bool InputManager::saveHistory(const std::string& filename) {
    if (filename.empty()) return false;
    
    // Ensure directory exists
    std::filesystem::path histPath(filename);
    std::filesystem::create_directories(histPath.parent_path());
    
    return linenoiseHistorySave(filename.c_str()) == 0;
}

bool InputManager::loadHistory(const std::string& filename) {
    if (filename.empty()) return false;
    
    return linenoiseHistoryLoad(filename.c_str()) == 0;
}

void InputManager::completionCallback(const char* input, linenoiseCompletions* completions) {
    if (!shell_instance) {
        return; 
    }
    
    std::string inputStr(input);
    auto suggestions = shell_instance->autocomplete(inputStr, inputStr.length());

    // add each suggestion to linenoise
    for (const auto& suggestion : suggestions) {
        linenoiseAddCompletion(completions, suggestion.c_str());
    }
}

InputManager::~InputManager() {
    
}

}
