#pragma once

#include <stdexcept>
#include <string>
#include <vector>

#include "token.hpp"

class Lexer {
  public:
    explicit Lexer(const std::string& source) : _source(source) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;

        while (_pos < _source.size()) {
            // newline
            if (current() == '\n') {
                tokens.push_back(newline());
                continue;
            }

            // whitespace
            if (std::isspace(current())) {
                advance();
                continue;
            }

            // comment
            if (current() == ';') {
                tokens.push_back(comment());
                continue;
            }

            // colon
            if (current() == ':') {
                tokens.push_back(colon());
                continue;
            }
             
            // identifier
            if (std::isalpha(current()) || current() == '_') {
                tokens.push_back(identifier());
                continue;
            }

            // number
            if (std::isdigit(current())) {
                tokens.push_back(number());
                continue;
            }

            throw std::runtime_error(
                "Unexpected character: " + std::string(1, current())
            );
        }

        tokens.push_back(end_of_file());

        return tokens;
    }

  private:
    const std::string& _source;
    std::size_t _pos = 0;
    std::size_t _line = 1;

    const char current() const { return _source[_pos]; }

    const char peek() const {
        if (_pos + 1 >= _source.size()) {
            return _source.back();
        }

        return _source[_pos + 1];
    }

    void advance() {
        if (_pos < _source.size()) _pos++;
    }

    Token newline() {
        Token newline{.type = TokenType::NEWLINE, .value = "\\n", .line = _line};

        advance(); // newline
        _line++;

        return newline;
    }

    Token comment() {
        Token comment{.type = TokenType::COMMENT, .line = _line};

        while (_pos < _source.size() && current() != '\n') {
            comment.value += current();
            advance();
        }

        return comment;
    }

    Token colon() {
        Token colon{.type = TokenType::COLON, .value = ":", .line = _line};

        advance(); // newline

        return colon;
    }

    Token identifier() {
        Token identifier{.type = TokenType::IDENTIFIER, .line = _line};

        while (_pos < _source.size() && (std::isalnum(current())) || current() == '_') {
            identifier.value += current();
            advance();
        }

        return identifier;
    }

    Token number() {
        Token number{.type = TokenType::NUMBER, .line = _line};

        // hexadecimal
        if (current() == '0' && (peek() == 'x' || peek() == 'X')) {
            number.type = TokenType::HEX_NUMBER;

            number.value += current();
            advance();

            number.value += current();
            advance();

            while (_pos < _source.size() && std::isxdigit(current())) {
                number.value += current();
                advance();
            }

            return number;
        }

        // decimal
        while (_pos < _source.size() && std::isdigit(current())) {
            number.value += current();
            advance();
        }

        return number;
    }

    Token end_of_file() const { return {.type = TokenType::END_OF_FILE, .value = ""}; }
};