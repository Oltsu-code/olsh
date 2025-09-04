#ifndef PARSER_H
#define PARSER_H

#include <memory>
#include <vector>
#include "tokenizer.h"
#include "ast.h"

namespace olsh {

class CommandParser {
private:
    std::vector<Parser::Token> tokens;
    size_t current;

    const Parser::Token& peek();
    const Parser::Token& advance();
    bool match(Parser::TokenType type);
    std::unique_ptr<Parser::Command> parseCommand();
    std::unique_ptr<Parser::Pipeline> parsePipeline();
    std::unique_ptr<Parser::ASTNode> parseRedirection(std::unique_ptr<Parser::ASTNode> node);

public:
    CommandParser();
    std::unique_ptr<Parser::ASTNode> parse(const std::string& input);
};

} // namespace olsh

#endif //PARSER_H
