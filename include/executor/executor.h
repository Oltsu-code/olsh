#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <memory>
#include "../parser/ast.h"
#include "../builtins/cd.h"
#include "../builtins/ls.h"
#include "../builtins/pwd.h"
#include "../builtins/echo.h"
#include "../builtins/rm.h"
#include "../builtins/help.h"
#include "../builtins/cat.h"
#include "../builtins/clear.h"
#include "../builtins/history.h"
#include "../builtins/joke.h"
#include "../builtins/alias.h"

namespace olsh {

class Executor {
private:
    // builtins
    Builtins::Cd cdCommand;
    Builtins::Ls lsCommand;
    Builtins::Pwd pwdCommand;
    Builtins::Echo echoCommand;
    Builtins::Rm rmCommand;
    Builtins::Help helpCommand;
    Builtins::Cat catCommand;
    Builtins::Clear clearCommand;
    Builtins::History historyCommand;
    Builtins::Joke jokeCommand;
    Builtins::Alias aliasCommand;

    int executeCommand(const Parser::Command& cmd);
    int executePipeline(const Parser::Pipeline& pipeline);
    int executeRedirection(const Parser::Redirection& redirection);
    int executeExternal(const Parser::Command& cmd);

public:
    Executor();
    int execute(std::unique_ptr<Parser::ASTNode> node);
};

}

#endif //EXECUTOR_H
