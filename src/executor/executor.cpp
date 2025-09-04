#include "../../include/executor/executor.h"
#include "../../include/executor/process.h"
#include "../../include/parser/ast.h"
#include "../../include/parser/parser.h"
#include "../../include/builtins/builtin_registry.h"
#include <utils/colors.h>
#include <iostream>
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#endif
#include <cstdio>
#include <string>

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
    try { // for some reason it crashes without the try
        for (const auto& cmd : pipeline.commands) {
            result = executeCommand(*cmd);
        }
    } catch (const std::exception& e) {
        std::cerr << RED << "Pipeline error: " << e.what() << RESET << std::endl;
        result = 1;
    }
    return result;
}

int Executor::executeRedirection(const Parser::Redirection& redirection) {
    int target = redirection.input ? 0 : 1; // stdin or stdout
    int file_fd = -1;
#ifdef _WIN32
    if (redirection.input) {
        file_fd = _open(redirection.filename.c_str(), _O_RDONLY);
    } else {
        int flags = redirection.append ? (_O_WRONLY | _O_CREAT | _O_APPEND)
                                       : (_O_WRONLY | _O_CREAT | _O_TRUNC);
        file_fd = _open(redirection.filename.c_str(), flags, _S_IREAD | _S_IWRITE);
    }
#else
    if (redirection.input) {
        file_fd = open(redirection.filename.c_str(), O_RDONLY);
    } else {
        int flags = redirection.append ? (O_WRONLY | O_CREAT | O_APPEND)
                                       : (O_WRONLY | O_CREAT | O_TRUNC);
        file_fd = open(redirection.filename.c_str(), flags, 0644);
    }
#endif
    if (file_fd == -1) {
        std::cerr << RED << "redirection: failed to open file: " << redirection.filename << RESET << std::endl;
        return 1;
    }

    int saved = -1;
#ifdef _WIN32
    saved = _dup(target);
    _dup2(file_fd, target);
    _close(file_fd);
#else
    saved = dup(target);
    dup2(file_fd, target);
    close(file_fd);
#endif

    int result = 0;
    switch (redirection.command->getType()) {
        case Parser::CommandType::BUILTIN:
        case Parser::CommandType::EXTERNAL:
            result = executeCommand(*static_cast<Parser::Command*>(redirection.command.get()));
            break;
        case Parser::CommandType::PIPELINE:
            result = executePipeline(*static_cast<Parser::Pipeline*>(redirection.command.get()));
            break;
        case Parser::CommandType::REDIRECTION:
            result = executeRedirection(*static_cast<Parser::Redirection*>(redirection.command.get()));
            break;
        default:
            std::cerr << RED << "redirection: unknown node" << RESET << std::endl;
            result = 1;
            break;
    }

#ifdef _WIN32
    _dup2(saved, target);
    _close(saved);
#else
    dup2(saved, target);
    close(saved);
#endif

    return result;
}

} // namespace olsh