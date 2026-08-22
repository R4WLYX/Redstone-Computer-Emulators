#pragma once

#include <string>
#include <vector>

#include "token.hpp"

enum class StatementType { INSTRUCTION, LABEL };

struct Statement {
    StatementType type;

    std::string name;
    std::vector<Token> operands;

    std::size_t line;
};