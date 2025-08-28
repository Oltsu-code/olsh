#include "../../include/executor/executor.h"
#include "../../include/executor/process.h"
#include "../../include/parser/ast.h"
#include "../../include/parser/parser.h"
#include "../../include/builtins/builtin_registry.h"
#include <utils/colors.h>
#include <iostream>

namespace olsh {

Executor::Executor() {}

int Executor::execute(std::unique_ptr<Parser::ASTNode> node) {
    if (!node) {
        std::cerr << RED << "Error: Invalid command\n" << RESET;
        return 1;
    }

    switch (node->getType()) {
        case Parser::CommandType::BUILTIN:
        case Parser::CommandType::EXTERNAL:
            return executeCommand(*static_cast<Parser::Command*>(node.get()));
        case Parser::CommandType::PIPELINE:
            return executePipeline(*static_cast<Parser::Pipeline*>(node.get()));
        case Parser::CommandType::REDIRECTION:
            return executeRedirection(*static_cast<Parser::Redirection*>(node.get()));
        default:
            std::cerr << RED << "Error: Unknown command type\n" << RESET;
            return 1;
    }
}

int Executor::executeCommand(const Parser::Command& cmd) {
    // builtins
    if (cmd.getType() == Parser::CommandType::BUILTIN) {
        return getBuiltinRegistry().execute(cmd.name, cmd.args);
    }

    // externals
    return executeExternal(cmd);
}

int Executor::executeExternal(const Parser::Command& cmd) {
    Process process;
    return process.execute(cmd.name, cmd.args);
}

int Executor::executePipeline(const Parser::Pipeline& pipeline) {
    // for now executed sequentally
    // TODO: make this better (idk how pls help)
    int result = 0;
    for (const auto& cmd : pipeline.commands) {
        result = executeCommand(*cmd);
    }
    return result;
}

int Executor::executeRedirection(const Parser::Redirection& redirection) {
    // TODO: add real redirection
    std::cout << "Redirection to " << redirection.filename
              << " (append: " << redirection.append
              << ", input: " << redirection.input << ")\n";

    return execute(std::unique_ptr<Parser::ASTNode>(redirection.command.get()));
}

}