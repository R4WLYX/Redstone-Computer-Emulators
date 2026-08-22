#pragma once

#include <string>

enum class TokenType { IDENTIFIER, NUMBER, HEX_NUMBER, COLON, COMMENT, NEWLINE, END_OF_FILE };

struct Token {
    TokenType type;
    std::string value;
    
    std::size_t line;
};
