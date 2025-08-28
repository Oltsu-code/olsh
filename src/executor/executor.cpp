#include "../../include/executor/executor.h"
#include "../../include/executor/process.h"
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
    if (cmd.getType() == Parser::CommandType::BUILTIN) {
        // exceute the builtins TODO: make this with a better way then whatever ts is

        if (cmd.name == "cd") return cdCommand.execute(cmd.args);
        if (cmd.name == "ls") return lsCommand.execute(cmd.args);
        if (cmd.name == "pwd") return pwdCommand.execute(cmd.args);
        if (cmd.name == "echo") return echoCommand.execute(cmd.args);
        if (cmd.name == "rm") return rmCommand.execute(cmd.args);
        if (cmd.name == "cat") return catCommand.execute(cmd.args);
        if (cmd.name == "clear") return clearCommand.execute(cmd.args);
        if (cmd.name == "history") return historyCommand.execute(cmd.args);
        if (cmd.name == "alias") return aliasCommand.execute(cmd.args);

        // if none of those match.
        std::cerr << RED
                  << "Error: Unknown builtin command: " << cmd.name << "\n"
                  << "This is probably a problem the parser. Please open a issue for this.\n"
                  << RESET;
        return 1;
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