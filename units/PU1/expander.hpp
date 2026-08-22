#pragma once

#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "statement.hpp"

class Expander {
  public:
    explicit Expander(const std::vector<Statement>& statements)
        : _statements(statements) {}

    std::vector<Statement> expand() {
        collect_labels();

        std::vector<Statement> result;

        for (std::size_t i = 0; i < _statements.size(); ++i) {
            const auto& statement = _statements[i];

            if (statement.type == StatementType::INSTRUCTION
                && statement.name == "CALL") {
                if (statement.operands.size() != 1) {
                    throw std::runtime_error("CALL expects one operand");
                }

                const auto& operand = statement.operands[0];

                if (operand.type != TokenType::IDENTIFIER) {
                    throw std::runtime_error("CALL expects a label");
                }

                auto it = _labels.find(operand.value);

                if (it == _labels.end()) {
                    throw std::runtime_error("Unknown label: " + operand.value);
                }

                std::size_t function_start = it->second + 1;

                for (std::size_t j = function_start; j < _statements.size(); ++j) {
                    if (_statements[j].type == StatementType::LABEL) break;
                    
                    result.push_back(_statements[j]);
                }

                continue;
            }

            result.push_back(statement);
        }

        return result;
    }

  private:
    const std::vector<Statement>& _statements;

    std::unordered_map<std::string, std::size_t> _labels;

    void collect_labels() {
        for (size_t i = 0; i < _statements.size(); ++i) {
            const auto& statement = _statements[i];

            if (statement.type == StatementType::LABEL) _labels[statement.name] = i;
        }
    }
};