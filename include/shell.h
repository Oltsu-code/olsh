#ifndef SHELL_H
#define SHELL_H

#include <string>
#include <memory>
#include <atomic>
#include "parser/parser.h"
#include "executor/executor.h"
#include "utils/autocomplete.h"
#include "utils/script.h"
#include "utils/config.h"
#include "utils/input_manager.h"
#include "builtins/alias.h"
#include "builtins/history.h"

namespace olsh {

namespace Utils { class ScriptInterpreter; }

class Shell {
private:
    std::unique_ptr<CommandParser> parser;
    std::unique_ptr<Executor> executor;
    std::unique_ptr<Utils::ScriptInterpreter> scriptInterpreter;
    std::unique_ptr<Builtins::Alias> aliasManager;
    std::unique_ptr<Builtins::History> historyManager;
    std::unique_ptr<Utils::Config> configManager;
    std::unique_ptr<Utils::InputManager> inputManager;
    std::unique_ptr<Utils::Autocomplete> autocompleteManager;
    std::string currentDirectory;
    bool running;

    void displayPrompt();
    std::string getPromptString();
    std::string expandPromptVariables(const std::string& promptTemplate);

    static std::atomic<bool> s_interrupted;

public:
    Shell();
    ~Shell();
    void run();
    void exit();
    int processCommand(const std::string& input);

    // Autocomplete interface for input manager
    std::vector<std::string> autocomplete(const std::string& input, size_t cursorPos);

    // Configuration access methods
    Utils::Config* getConfigManager() const { return configManager.get(); }

    // Called by platform-specific signal handlers to record an interrupt event
    static void notifyInterrupted();
};

}

#endif //SHELL_H
