#pragma once
#include <string>
#include <memory>
#include "readline.h"

namespace olsh {
    class Shell;
} // namespace olsh

namespace olsh::Utils {

class InputManager {
private:
    static std::unique_ptr<InputManager> instance;
    static olsh::Shell* shell_instance;
    
public:
    InputManager();
    
    static InputManager& getInstance();
    static void setShellInstance(olsh::Shell* shell);
    
    // core input functions
    std::string readLine(const std::string& prompt);
    
    // tab compleation
    static void completionCallback(const char* input, readlineCompletions* completions);
    
    // history management
    void addToHistory(const std::string& line);
    bool saveHistory(const std::string& filename);
    bool loadHistory(const std::string& filename);
    
    ~InputManager();
};

} // namespace olsh::Utils
