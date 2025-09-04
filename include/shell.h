#ifndef SHELL_H
#define SHELL_H

#include <string>
#include <memory>
#include <atomic>

#ifdef _WIN32
#include <windows.h>
#endif

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

    // signal handling
    static void setupSignalHandlers();
    static void cleanupSignalHandlers();
    
#ifdef _WIN32
    static BOOL WINAPI signalHandler(DWORD dwCtrlType);
#else
    static void signalHandler(int signal);
#endif

public:
    Shell();
    ~Shell();
    void run();
    void exit();
    int processCommand(const std::string& input);

    std::vector<std::string> autocomplete(const std::string& input, size_t cursorPos);
    Utils::Config* getConfigManager() const { return configManager.get(); }
    static void notifyInterrupted();
};

} // namespace olsh

#endif //SHELL_H
