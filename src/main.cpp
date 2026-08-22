#include <iostream>

#include "PU1/PU1.hpp"

int main() {
    try {
        PU1 unit;
        unit.load("res/fib.pu1");

        auto byte_code = unit.byte_code();
        std::size_t byte_len = 0;

        for (std::size_t i = 0; i < byte_code.size(); i++) {
            Word word = byte_code[i];
            Opcode opcode = static_cast<Opcode>(word.value);

            std::cout << std::hex << +word.value;

            if (opcode == Opcode::LOAD_3 && i + 1 < byte_code.size())
                std::cout << std::hex << +byte_code[++i].value;

            std::cout << '/';

            byte_len++;
        }

        std::cout << "\nInstruction count: " << std::dec << byte_len;

        std::cin.get();

        std::cout << '\n';

        while (!unit.halted()) {
            auto registers = unit.registers();
            auto modified = unit.last_modified_register();

            std::cout << "PC: " << unit.program_counter() << '\n';

            std::cout << (modified == 1 ? "> " : "  ") << "R1: " << +registers[0] << '\n';

            std::cout << (modified == 2 ? "> " : "  ") << "R2: " << +registers[1] << '\n';

            std::cout << (modified == 3 ? "> " : "  ") << "R3: " << +registers[2] << '\n';

            std::cout << (modified == 4 ? "> " : "  ") << "R4: " << +registers[3] << '\n';

            std::cout << '\n';

            // std::cin.get();

            unit.step();
        }

    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << '\n';
        return 1;
    }

    return 0;
}