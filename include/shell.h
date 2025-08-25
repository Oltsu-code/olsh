#ifndef SHELL_H
#define SHELL_H

#include <string>
#include <memory>
#include "parser/parser.h"
#include "executor/executor.h"
#include "utils/autocomplete.h"
#include "utils/script.h"
#include "builtins/alias.h"
#include "builtins/history.h"

namespace olsh {

namespace Utils { class ScriptInterpreter; }

class Shell {
private:
    std::unique_ptr<CommandParser> parser;
    std::unique_ptr<Executor> executor;
    std::unique_ptr<Utils::Autocomplete> autocomplete;
    std::unique_ptr<Utils::ScriptInterpreter> scriptInterpreter;
    std::unique_ptr<Builtins::Alias> aliasManager;
    std::unique_ptr<Builtins::History> historyManager;
    std::string currentDirectory;
    bool running;

    void displayPrompt();
    std::string readInputWithAutocomplete();
    std::string readInput();
    void handleTabCompletion(std::string& input, size_t& cursorPos);

public:
    Shell();
    ~Shell();
    void run();
    void exit();
    void processCommand(const std::string& input);
};

}

#endif //SHELL_H
