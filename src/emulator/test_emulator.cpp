#include "cpu.hpp"
#include <iostream>
#include <vector>

// Helper function to create instruction words
uint16_t make_instruction(uint8_t opcode, uint8_t mode, uint8_t rd, uint8_t rs) {
    return static_cast<uint16_t>(
        (static_cast<uint16_t>(opcode & 0x1F) << 11) |
        (static_cast<uint16_t>(mode & 0x07) << 8) |
        (static_cast<uint16_t>(rd & 0x07) << 5) |
        (static_cast<uint16_t>(rs & 0x07) << 2)
    );
}

// Convert 16-bit word to little-endian bytes
void add_word_to_program(std::vector<uint8_t>& program, uint16_t word) {
    program.push_back(static_cast<uint8_t>(word & 0xFF));        // Low byte
    program.push_back(static_cast<uint8_t>((word >> 8) & 0xFF)); // High byte
}

int main() {
    std::cout << "=== Software CPU Emulator Test ===" << std::endl;
    
    // Create CPU instance
    CPU cpu;
    cpu.set_debug_mode(true);
    
    // Create a simple test program:
    // ADD R0, #10    ; R0 = R0 + 10 (R0 starts at 0, so R0 = 10)
    // ADD R0, #5     ; R0 = R0 + 5  (R0 = 15)
    // NOP            ; Do nothing
    // HALT           ; Stop execution
    
    std::vector<uint8_t> program;
    
    // Instruction 1: ADD R0, #10 (immediate mode)
    // Opcode: 5 (ADD), Mode: 1 (IMMEDIATE), RD: 0 (R0), RS: 0 (unused)
    uint16_t instr1 = make_instruction(5, 1, 0, 0);
    add_word_to_program(program, instr1);
    add_word_to_program(program, 10);  // Immediate value: 10
    
    // Instruction 2: ADD R0, #5 (immediate mode)
    uint16_t instr2 = make_instruction(5, 1, 0, 0);
    add_word_to_program(program, instr2);
    add_word_to_program(program, 5);   // Immediate value: 5
    
    // Instruction 3: NOP
    // Opcode: 0 (NOP), Mode: 0, RD: 0, RS: 0
    uint16_t instr3 = make_instruction(0, 0, 0, 0);
    add_word_to_program(program, instr3);
    
    // Instruction 4: HALT
    // Opcode: 1 (HALT), Mode: 0, RD: 0, RS: 0
    uint16_t instr4 = make_instruction(1, 0, 0, 0);
    add_word_to_program(program, instr4);
    
    std::cout << "\nProgram created (" << program.size() << " bytes):" << std::endl;
    std::cout << "1. ADD R0, #10" << std::endl;
    std::cout << "2. ADD R0, #5" << std::endl;
    std::cout << "3. NOP" << std::endl;
    std::cout << "4. HALT" << std::endl;
    
    // Load and run the program
    std::cout << "\n=== Loading Program ===" << std::endl;
    cpu.load_program(program, 0x8000);
    
    std::cout << "\n=== Initial CPU State ===" << std::endl;
    cpu.dump_state();
    
    std::cout << "\n=== Running Program ===" << std::endl;
    cpu.run();
    
    std::cout << "\n=== Final CPU State ===" << std::endl;
    cpu.dump_state();
    
    // Verify results
    const Registers& regs = cpu.get_registers();
    uint16_t r0_value = regs.get_gpr(0);
    
    std::cout << "\n=== Test Results ===" << std::endl;
    std::cout << "Expected R0 value: 15" << std::endl;
    std::cout << "Actual R0 value: " << r0_value << std::endl;
    
    if (r0_value == 15) {
        std::cout << "✅ TEST PASSED!" << std::endl;
    } else {
        std::cout << "❌ TEST FAILED!" << std::endl;
        return 1;
    }
    
    std::cout << "\n=== Emulator Test Complete ===" << std::endl;
    return 0;
}