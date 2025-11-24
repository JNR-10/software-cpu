#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include "emulator/cpu.hpp"
#include "assembler/assembler.hpp"

int main() {
    // Countdown loop demo in assembly (no labels, using byte addresses).
    // Each instruction is 4 bytes, so instruction N starts at address 4 * N.
    // Layout:
    //   addr 0:  LOADI R0, 3       ; R0 = 3  (loop counter)
    //   addr 4:  LOADI R1, 1       ; R1 = 1  (amount to subtract)
    //   addr 8:  SUB   R0, R1      ; LOOP: R0 = R0 - 1, updates ZR flag
    //   addr 12: JZ    20          ; if ZR == 1 (R0 == 0), jump to END at 20
    //   addr 16: JMP   8           ; otherwise, jump back to LOOP at 8
    //   addr 20: HLT               ; END: stop execution
    // This means the loop executes exactly 3 times, taking R0 from 3 -> 2 -> 1 -> 0.
    std::string source =
        "LOADI R0, 3\n"
        "LOADI R1, 1\n"
        "SUB R0, R1\n"
        "JZ 20\n"   // correct END address for HLT
        "JMP 8\n"
        "HLT\n";

    std::vector<std::uint8_t> program = assemble(source);

    CPU cpu;
    cpu.load_program(program.data(), program.size(), 0x0000);

    cpu.run();

    const Registers& regs = cpu.regs();
    std::cout << "R0 = " << regs.R0 << "\n";
    std::cout << "R1 = " << regs.R1 << "\n";
    std::cout << "ZR = " << regs.ZR << "\n";
    std::cout << "PC = " << regs.PC << "\n";
    std::cout << "halted = " << regs.halted << "\n";

    return 0;
}