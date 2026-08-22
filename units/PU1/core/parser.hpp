#pragma once

#include <stdexcept>
#include <vector>

#include "statement.hpp"
#include "token.hpp"

class Parser {
  public:
    explicit Parser(const std::vector<Token>& tokens) : _tokens(tokens) {}

    std::vector<Statement> parse() {
        std::vector<Statement> statements;

        while (current().type != TokenType::END_OF_FILE) {
            // empty line
            if (current().type == TokenType::NEWLINE) {
                advance();
                continue;
            }

            // comment
            if (current().type == TokenType::COMMENT) {
                advance();

                if (current().type == TokenType::NEWLINE) {
                    advance();
                }

                continue;
            }

            // label
            if (current().type == TokenType::IDENTIFIER
                && peek().type == TokenType::COLON) {
                statements.push_back(parse_label());
                continue;
            }

            // instruction
            if (current().type == TokenType::IDENTIFIER) {
                statements.push_back(parse_instruction());
                continue;
            }

            throw std::runtime_error("Unexpected token: " + current().value);
        }

        return statements;
    }

  private:
    const std::vector<Token>& _tokens;
    std::size_t _pos = 0;

    const Token& current() const { return _tokens[_pos]; }

    const Token& peek() const {
        if (_pos + 1 >= _tokens.size()) {
            return _tokens.back();
        }

        return _tokens[_pos + 1];
    }

    void advance() {
        if (_pos < _tokens.size()) _pos++;
    }

    Statement parse_label() {
        Statement label{
            .type = StatementType::LABEL, .name = current().value, .line = current().line
        };

        advance(); // identifier
        advance(); // colon

        if (current().type == TokenType::NEWLINE) {
            advance();
        }

        return label;
    }

    Statement parse_instruction() {
        Statement instruction{
            .type = StatementType::INSTRUCTION,
            .name = current().value,
            .line = current().line
        };

        advance(); // instruction name

        while (current().type != TokenType::NEWLINE
               && current().type != TokenType::END_OF_FILE) {
            instruction.operands.push_back(current());
            advance();
        }

        if (current().type == TokenType::NEWLINE) {
            advance();
        }

        return instruction;
    }
};