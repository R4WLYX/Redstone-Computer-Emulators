#pragma once

#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "statement.hpp"
#include "token.hpp"

enum class WordType { INSTRUCTION, DATA };

struct Word {
    WordType type;
    std::uint8_t value;
};

// clang-format off
enum class Opcode : std::uint8_t {
    SET       = 0x1,
    CLEAR_3   = 0x2,
    SUB       = 0x3,
    MERGE_2_4 = 0x4,
    CLEAR_1   = 0x5,
    MERGE_1_3 = 0x7,
    CLEAR_2   = 0x8,
    MERGE_1_2 = 0x9,
    MERGE_3_1 = 0xa,
    LOAD_3    = 0xd,
    HALT      = 0xf
};
// clang-format on

class CompilerError : public std::runtime_error {
  public:
    CompilerError(std::size_t line, const std::string& message)
        : std::runtime_error("line " + std::to_string(line) + ": " + message) {}
};

class Compiler {
  public:
    explicit Compiler(const std::vector<Statement>& statements)
        : _statements(statements) {}

    std::vector<Word> compile() {
        std::vector<Word> output;

        for (const auto& statement : _statements) {
            if (statement.type == StatementType::LABEL) {
                continue;
            }

            if (statement.name == "CLEAR") {
                expect_operands(statement, 1);

                std::uint8_t reg = parse_register(statement.operands[0]);

                auto it = clear_opcodes.find(reg);

                if (it == clear_opcodes.end()) {
                    error(statement, "CLEAR does not support R" + std::to_string(reg));
                }

                output.push_back(instruction(it->second));

            } else if (statement.name == "SET") {
                expect_operands(statement, 0);

                output.push_back(instruction(Opcode::SET));

            } else if (statement.name == "MERGE") {
                expect_operands(statement, 2);

                std::uint8_t reg_a = parse_register(statement.operands[0]);
                std::uint8_t reg_b = parse_register(statement.operands[1]);

                std::uint16_t key = (reg_a << 8) | reg_b;

                auto it = merge_opcodes.find(key);

                if (it == merge_opcodes.end()) {
                    error(
                        statement,
                        "MERGE does not support R" + std::to_string(reg_a) + " R"
                            + std::to_string(reg_b)
                    );
                }

                output.push_back(instruction(it->second));

            } else if (statement.name == "SUB") {
                expect_operands(statement, 0);

                output.push_back(instruction(Opcode::SUB));

            } else if (statement.name == "LOAD") {
                expect_operands(statement, 1);

                std::uint8_t value = parse_number(statement.operands[0]);

                if (value > 0xF) {
                    error(
                        statement.operands[0],
                        "LOAD immediate must be a single hex digit (0x0-0xF)"
                    );
                }

                output.push_back(instruction(Opcode::LOAD_3));
                output.push_back(data(value));

            } else if (statement.name == "HALT") {
                expect_operands(statement, 0);

                output.push_back(instruction(Opcode::HALT));

            } else {
                throw std::runtime_error("Unknown instruction: " + statement.name);
            }
        }

        return output;
    }

  private:
    const std::vector<Statement>& _statements;

    inline static const std::unordered_map<std::uint16_t, Opcode> clear_opcodes = {
        {1, Opcode::CLEAR_1},
        {2, Opcode::CLEAR_2},
        {3, Opcode::CLEAR_3},
    };

    inline static const std::unordered_map<std::uint16_t, Opcode> merge_opcodes = {
        {(2 << 8) | 4, Opcode::MERGE_2_4},
        {(1 << 8) | 3, Opcode::MERGE_1_3},
        {(1 << 8) | 2, Opcode::MERGE_1_2},
        {(3 << 8) | 1, Opcode::MERGE_3_1},
    };

    Word instruction(Opcode opcode) {
        return {
            .type = WordType::INSTRUCTION, .value = static_cast<std::uint8_t>(opcode)
        };
    }

    Word data(std::uint64_t value) {
        return {.type = WordType::DATA, .value = static_cast<std::uint8_t>(value)};
    }

    std::uint8_t parse_register(const Token& token) {
        if (token.type != TokenType::IDENTIFIER) {
            error(token, "Expected register, got '" + token.value + "'");
        }

        if (token.value.size() < 2 || token.value[0] != 'R') {
            error(token, "Expected register, got '" + token.value + "'");
        }

        for (std::size_t i = 1; i < token.value.size(); ++i) {
            if (!std::isdigit(static_cast<unsigned char>(token.value[i]))) {
                error(token, "Invalid register '" + token.value + "'");
            }
        }

        auto number = std::stoul(token.value.substr(1));

        if (number < 1 || number > 4) {
            error(token, "Unknown register '" + token.value + "'");
        }

        return static_cast<std::uint8_t>(number);
    }

    std::uint8_t parse_number(const Token& token) {
        if (token.type != TokenType::NUMBER && token.type != TokenType::HEX_NUMBER) {
            error(token, "Expected number, got '" + token.value + "'");
        }

        try {
            auto value = std::stoull(token.value, nullptr, 0);

            if (value > 0xFF) {
                error(token, "Number '" + token.value + "' is too large");
            }

            return static_cast<std::uint8_t>(value);
        } catch (const std::exception&) {
            error(token, "Invalid number '" + token.value + "'");
        }
    }

    [[noreturn]]
    void error(const Token& token, const std::string& message) {
        throw CompilerError(token.line, message);
    }

    [[noreturn]]
    void error(const Statement& statement, const std::string& message) {
        throw CompilerError(statement.line, message);
    }

    void expect_operands(const Statement& statement, std::size_t expected) {
        if (statement.operands.size() != expected) {
            error(
                statement,
                statement.name + " expects " + std::to_string(expected) + " operand"
                    + (expected == 1 ? "" : "s") + ", got "
                    + std::to_string(statement.operands.size())
            );
        }
    }
};