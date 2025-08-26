#include "../../include/parser/ast.h"

namespace olsh::Parser {

Command::Command(const std::string& cmdName, const std::vector<std::string>& arguments)
    : name(cmdName), args(arguments) {}

CommandType Command::getType() const {
    // builtins
    if (name == "cd" || name == "ls" || name == "pwd" || name == "echo" ||
        name == "rm" || name == "cat" || name == "clear" ||
        name == "history" || name == "alias") {
        return CommandType::BUILTIN;
    }
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