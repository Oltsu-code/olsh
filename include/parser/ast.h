#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <memory>

namespace olsh::Parser {

enum class CommandType {
    BUILTIN,
    EXTERNAL,
    PIPELINE,
    REDIRECTION
};

class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual CommandType getType() const = 0;
};

class Command : public ASTNode {
public:
    std::string name;
    std::vector<std::string> args;

    Command(const std::string& cmdName, const std::vector<std::string>& arguments = {});
    CommandType getType() const override;
};

class Pipeline : public ASTNode {
public:
    std::vector<std::unique_ptr<Command>> commands;

    Pipeline(std::vector<std::unique_ptr<Command>> cmds);
    CommandType getType() const override;
};

class Redirection : public ASTNode {
public:
    std::unique_ptr<ASTNode> command;
    std::string filename;
    bool append;
    bool input;

    Redirection(std::unique_ptr<ASTNode> cmd, const std::string& file, bool isAppend = false, bool isInput = false);
    CommandType getType() const override;
};

}

#endif //AST_H
