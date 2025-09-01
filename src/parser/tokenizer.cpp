#include "../../include/parser/tokenizer.h"
#include <cctype>

namespace olsh::Parser {

Tokenizer::Tokenizer(const std::string& input)
    : input(input), position(0), length(input.length()) {}

char Tokenizer::currentChar() {
    if (position >= length) return '\0';
    return input[position];
}

char Tokenizer::peek() {
    if (position + 1 >= length) return '\0';
    return input[position + 1];
}

void Tokenizer::advance() {
    if (position < length) position++;
}

void Tokenizer::skipWhitespace() {
    while (position < length && std::isspace(currentChar())) {
        advance();
    }
}

std::string Tokenizer::readWord() {
    std::string word;
    while (position < length && !std::isspace(currentChar()) &&
           currentChar() != '|' && currentChar() != '>' &&
           currentChar() != '<' && currentChar() != ';') {
        word += currentChar();
        advance();
    }
    return word;
}

std::string Tokenizer::readQuotedString(char quote) {
    std::string str;
    advance(); // skip opening quote

    while (position < length && currentChar() != quote) {
        if (currentChar() == '\\' && peek() == quote) {
            advance(); // skip \ .
            str += currentChar();
            advance();
        } else {
            str += currentChar();
            advance();
        }
    }

    if (position < length) advance(); // skip closing quote
    return str;
}

std::vector<Token> Tokenizer::tokenize() {
    std::vector<Token> tokens;

    while (position < length) {
        skipWhitespace();

        if (position >= length) break;

        char ch = currentChar();

        switch (ch) {
            case '|':
                tokens.emplace_back(TokenType::PIPE, "|");
                advance();
                break;
            case '>':
                if (peek() == '>') {
                    tokens.emplace_back(TokenType::REDIRECT_APPEND, ">>");
                    advance();
                    advance();
                } else {
                    tokens.emplace_back(TokenType::REDIRECT_OUT, ">");
                    advance();
                }
                break;
            case '<':
                tokens.emplace_back(TokenType::REDIRECT_IN, "<");
                advance();
                break;
            case ';':
                tokens.emplace_back(TokenType::SEMICOLON, ";");
                advance();
                break;
            case '"':
            case '\'':
                tokens.emplace_back(TokenType::WORD, readQuotedString(ch));
                break;
            case '&':
                tokens.emplace_back(TokenType::AMPERSAND, "&");
                advance();
                break;
            default:
                if (std::isalnum(ch) || ch == '.' || ch == '/' || ch == '\\' ||
                    ch == '-' || ch == '_' || ch == '~' || ch == '*' || ch == '?') {
                    tokens.emplace_back(TokenType::WORD, readWord());
                } else {
                    advance(); // Skip unknown characters
                }
                break;
        }
    }

    tokens.emplace_back(TokenType::END_OF_INPUT, "");
    return tokens;
}

}