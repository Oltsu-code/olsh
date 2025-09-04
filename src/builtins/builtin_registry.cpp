#include "../../include/builtins/builtin_registry.h"
#include "../../include/builtins/cd.h"
#include "../../include/builtins/ls.h"
#include "../../include/builtins/pwd.h"
#include "../../include/builtins/echo.h"
#include "../../include/builtins/rm.h"
#include "../../include/builtins/cat.h"
#include "../../include/builtins/clear.h"
#include "../../include/builtins/history.h"
#include "../../include/builtins/alias.h"
#include "../../include/builtins/config.h"
#include "../../include/builtins/mkdir.h"
#include "../../include/builtins/cp.h"

namespace olsh {

BuiltinRegistry::BuiltinRegistry() {
    registerCommands();
}

void BuiltinRegistry::registerCommands() {
    Builtins::Cd cdCommand;
    Builtins::Ls lsCommand;
    Builtins::Pwd pwdCommand;
    Builtins::Echo echoCommand;
    Builtins::Rm rmCommand;
    Builtins::Cat catCommand;
    Builtins::Clear clearCommand;
    Builtins::History historyCommand;
    Builtins::Alias aliasCommand;
    Builtins::Config configCommand;
    Builtins::Mkdir mkdirCommand;
    Builtins::Cp cpCommand;

    commands["cd"] = [cdCommand](const std::vector<std::string>& args) mutable { return cdCommand.execute(args); };
    commands["ls"] = [lsCommand](const std::vector<std::string>& args) mutable { return lsCommand.execute(args); };
    commands["pwd"] = [pwdCommand](const std::vector<std::string>& args) mutable { return pwdCommand.execute(args); };
    commands["echo"] = [echoCommand](const std::vector<std::string>& args) mutable { return echoCommand.execute(args); };
    commands["rm"] = [rmCommand](const std::vector<std::string>& args) mutable { return rmCommand.execute(args); };
    commands["cat"] = [catCommand](const std::vector<std::string>& args) mutable { return catCommand.execute(args); };
    commands["clear"] = [clearCommand](const std::vector<std::string>& args) mutable { return clearCommand.execute(args); };
    commands["history"] = [historyCommand](const std::vector<std::string>& args) mutable { return historyCommand.execute(args); };
    commands["alias"] = [aliasCommand](const std::vector<std::string>& args) mutable { return aliasCommand.execute(args); };
    commands["config"] = [configCommand](const std::vector<std::string>& args) mutable { return configCommand.execute(args); };
    commands["mkdir"] = [mkdirCommand](const std::vector<std::string>& args) mutable { return mkdirCommand.execute(args); };
    commands["cp"] = [cpCommand](const std::vector<std::string>& args) mutable { return cpCommand.execute(args); };
}

bool BuiltinRegistry::isBuiltin(const std::string& command) const {
    return commands.find(command) != commands.end();
}

int BuiltinRegistry::execute(const std::string& command, const std::vector<std::string>& args) const {
    auto it = commands.find(command);
    if (it != commands.end()) {
        return it->second(args);
    }
    return 1;
}

BuiltinRegistry& getBuiltinRegistry() {
    static BuiltinRegistry instance;
    return instance;
}

} // namespace olsh
