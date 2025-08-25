#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <vector>

namespace olsh::Parser {

enum class TokenType {
    WORD,
    PIPE,
    REDIRECT_OUT,
    REDIRECT_IN,
    REDIRECT_APPEND,
    SEMICOLON,
    END_OF_INPUT
};

struct Token {
    TokenType type;
    std::string value;

    Token(TokenType t, const std::string& v) : type(t), value(v) {}
};

class Tokenizer {
private:
    std::string input;
    size_t position;
    size_t length;

    char currentChar();
    char peek();
    void advance();
    void skipWhitespace();
    std::string readWord();
    std::string readQuotedString(char quote);

public:
    Tokenizer(const std::string& input);
    std::vector<Token> tokenize();
};

}

#endif //TOKENIZER_H
