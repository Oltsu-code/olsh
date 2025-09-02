#include "../../include/parser/ast.h"
#include "../../include/builtins/builtin_registry.h"

#include <iostream> //! debug remember to delete

namespace olsh::Parser {

Command::Command(const std::string& cmdName, const std::vector<std::string>& arguments, bool skipBuiltin)
    : name(cmdName), args(arguments), skipBuiltinLookup(skipBuiltin) {}

CommandType Command::getType() const {
    // builtin (skip if ^ is used)
    if (!skipBuiltinLookup && getBuiltinRegistry().isBuiltin(name)) {
        return CommandType::BUILTIN;
    }

    std::cout << "yo gang bangys we skipped the builtin wohoooo";

    // external
    return CommandType::EXTERNAL;
}

Pipeline::Pipeline(std::vector<std::unique_ptr<Command>> cmds)
    : commands(std::move(cmds)) {}

CommandType Pipeline::getType() const {
    return CommandType::PIPELINE;
}

Redirection::Redirection(std::unique_ptr<ASTNode> cmd, const std::string& file,
                        bool isAppend, bool isInput)
    : command(std::move(cmd)), filename(file), append(isAppend), input(isInput) {}

CommandType Redirection::getType() const {
    return CommandType::REDIRECTION;
}

}