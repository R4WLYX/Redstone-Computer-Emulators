#pragma once

#include <array>
#include <optional>

#include "compiler.hpp"
#include "expander.hpp"
#include "lexer.hpp"
#include "parser.hpp"

#include "../unit.hpp"

/*
0x1: SET               | wipe + write to reg4 from reg3
0x2: CLEAR R3          | wipe reg3
0x3: SUB               | sub reg3 by reg2
0x4: MERGE R2 R4       | write to reg2 from reg4
0x5: CLEAR R1          | wipe reg1
0x7: MERGE R1 R3       | write to reg1 from reg3
0x8: CLEAR R2          | wipe reg2
0x9: MERGE R1 R2       | write to reg1 from reg2
0xa: MERGE R3 R1       | write to reg3 from reg1
0xd: LOAD R3 immediate | write immediate to reg3
*/

class PU1 : public Unit {
  public:
    void load(const std::filesystem::path& path) {
        std::string source = read_file(path);

        Lexer lexer(source);
        auto tokens = lexer.tokenize();

        Parser parser(tokens);
        auto statements = parser.parse();

        Expander expander(statements);
        auto expanded = expander.expand();

        Compiler compiler(expanded);
        _byte_code = compiler.compile();
    }

    void run() {
        while (step()) {
        }
    }

    bool step() {
        if (_halted || _pc >= _byte_code.size()) return false;

        _last_modified_register.reset();

        const Word& word = _byte_code[_pc];

        if (word.type != WordType::INSTRUCTION) {
            throw std::runtime_error("Expected instruction");
        }

        Opcode opcode = static_cast<Opcode>(word.value);

        if (opcode == Opcode::HALT) {
            _halted = true;
            _pc++;

            return true;
        }

        _pc += execute(opcode);

        return true;
    }

    const std::vector<Word>& byte_code() const { return _byte_code; }

    const std::array<std::uint8_t, 4>& registers() const { return _registers; }

    std::size_t program_counter() const { return _pc; }

    bool halted() const { return _halted; }

    std::optional<std::uint8_t> last_modified_register() const {
        return _last_modified_register;
    }

  private:
    std::vector<Word> _byte_code;
    std::array<std::uint8_t, 4> _registers{};
    std::size_t _pc = 0;
    bool _halted = false;
    std::optional<std::uint8_t> _last_modified_register;

    std::uint8_t& reg(std::uint8_t number) { return _registers.at(number - 1); }

    void modified(std::uint8_t number) { _last_modified_register = number; }

    std::size_t execute(Opcode opcode) {
        switch (opcode) {
        case Opcode::SET:
            reg(4) = reg(3);
            modified(4);
            return 1;

        case Opcode::CLEAR_3:
            reg(3) = 0;
            modified(3);
            return 1;

        case Opcode::SUB:
            if (reg(3) < reg(2)) reg(3) = 0;
            else reg(3) -= reg(2);
            modified(3);
            return 1;

        case Opcode::MERGE_2_4:
            reg(2) = std::max(reg(2), reg(4));
            modified(2);
            return 1;

        case Opcode::CLEAR_1:
            reg(1) = 0;
            modified(1);
            return 1;

        case Opcode::MERGE_1_3:
            reg(1) = std::max(reg(1), reg(3));
            modified(1);
            return 1;

        case Opcode::CLEAR_2:
            reg(2) = 0;
            modified(2);
            return 1;

        case Opcode::MERGE_1_2:
            reg(1) = std::max(reg(1), reg(2));
            modified(1);
            return 1;

        case Opcode::MERGE_3_1:
            reg(3) = std::max(reg(3), reg(1));
            modified(3);
            return 1;

        case Opcode::LOAD_3:
            if (_pc + 1 >= _byte_code.size()) {
                throw std::runtime_error("LOAD missing immediate");
            }

            if (_byte_code[_pc + 1].type != WordType::DATA) {
                throw std::runtime_error("LOAD expected immediate data");
            }

            reg(3) = _byte_code[_pc + 1].value;
            modified(3);
            return 2;

        default:
            throw std::runtime_error("Unknown opcode");
        }
    }
};