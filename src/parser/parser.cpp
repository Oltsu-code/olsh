#include "../../include/parser/parser.h"
#include <utils/colors.h>
#include <iostream>

namespace olsh {

CommandParser::CommandParser() : current(0) {}

const Parser::Token& CommandParser::peek() {
    if (current >= tokens.size()) {
        static Parser::Token eoi(Parser::TokenType::END_OF_INPUT, "");
        return eoi;
    }
    return tokens[current];
}

const Parser::Token& CommandParser::advance() {
    if (current < tokens.size()) current++;
    return peek();
}

bool CommandParser::match(Parser::TokenType type) {
    if (peek().type == type) {
        advance();
        return true;
    }
    return false;
}

std::unique_ptr<Parser::Command> CommandParser::parseCommand() {
    if (peek().type != Parser::TokenType::WORD) {
        return nullptr;
    }

    std::string name = peek().value;
    advance();

    std::vector<std::string> args;
    while (peek().type == Parser::TokenType::WORD) {
        args.push_back(peek().value);
        advance();
    }

    return std::make_unique<Parser::Command>(name, args);
}

std::unique_ptr<Parser::Pipeline> CommandParser::parsePipeline() {
    std::vector<std::unique_ptr<Parser::Command>> commands;

    auto cmd = parseCommand();
    if (!cmd) return nullptr;

    commands.push_back(std::move(cmd));

    while (match(Parser::TokenType::PIPE)) {
        cmd = parseCommand();
        if (cmd) {
            commands.push_back(std::move(cmd));
        }
    }

    if (commands.size() == 1) {
        // return directly
        return nullptr;
    }

    return std::make_unique<Parser::Pipeline>(std::move(commands));
}

std::unique_ptr<Parser::ASTNode> CommandParser::parseRedirection(std::unique_ptr<Parser::ASTNode> node) {
    while (peek().type == Parser::TokenType::REDIRECT_OUT ||
           peek().type == Parser::TokenType::REDIRECT_APPEND ||
           peek().type == Parser::TokenType::REDIRECT_IN) {

        bool append = peek().type == Parser::TokenType::REDIRECT_APPEND;
        bool input = peek().type == Parser::TokenType::REDIRECT_IN;
        advance();

        if (peek().type != Parser::TokenType::WORD) {
            std::cerr << RED << "Expected filename after redirection\n" << RESET;
            return node;
        }

        std::string filename = peek().value;
        advance();

        node = std::make_unique<Parser::Redirection>(std::move(node), filename, append, input);
    }

    return node;
}

std::unique_ptr<Parser::ASTNode> CommandParser::parse(const std::string& input) {
    if (input.empty()) return nullptr;

    Parser::Tokenizer tokenizer(input);
    tokens = tokenizer.tokenize();
    current = 0;

    // try to parse as pipeline
    size_t saved_pos = current;
    auto pipeline = parsePipeline();
    if (pipeline) {
        return parseRedirection(std::move(pipeline));
    }

    // no pipeline
    current = saved_pos;
    auto command = parseCommand();
    if (command) {
        return parseRedirection(std::move(command));
    }

    return nullptr;
}

}
