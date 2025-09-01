#pragma once
#include <string>
#include <memory>
#include "linenoise.h"

namespace olsh {
    class Shell; // Forward declaration
}

namespace olsh::Utils {

// Smart input handling using linenoise for professional terminal experience
class InputManager {
private:
    static std::unique_ptr<InputManager> instance;
    static olsh::Shell* shell_instance; // For completion callbacks
    
public:
    InputManager(); // Made public for make_unique
    
    static InputManager& getInstance();
    static void setShellInstance(olsh::Shell* shell);
    
    // Core input functions - clean interface over linenoise
    std::string readLine(const std::string& prompt);
    
    // Tab completion bridge between linenoise and shell
    static void completionCallback(const char* input, linenoiseCompletions* completions);
    
    // History management
    void addToHistory(const std::string& line);
    bool saveHistory(const std::string& filename);
    bool loadHistory(const std::string& filename);
    
    ~InputManager();
};

}
