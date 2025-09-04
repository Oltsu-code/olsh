#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <memory>
#include "../parser/ast.h"

namespace olsh {

class Executor {
private:
    int executeCommand(const Parser::Command& cmd);
    int executePipeline(const Parser::Pipeline& pipeline);
    int executeRedirection(const Parser::Redirection& redirection);
    int executeExternal(const Parser::Command& cmd);

public:
    Executor();
    int execute(std::unique_ptr<Parser::ASTNode> node);
};

} // namespace olsh

#endif //EXECUTOR_H
