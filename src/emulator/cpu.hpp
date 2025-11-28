#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include "memory.hpp"
#include "registers.hpp"
#include "alu.hpp"

class CPU {
public:
    // Instruction opcodes from architecture specification
    enum class Opcode : uint8_t {
        NOP = 0,    HALT = 1,   MOV = 2,    LOAD = 3,   STORE = 4,
        ADD = 5,    SUB = 6,    AND = 7,    OR = 8,     XOR = 9,
        CMP = 10,   SHL = 11,   SHR = 12,   JMP = 13,   JZ = 14,
        JNZ = 15,   JC = 16,    JNC = 17,   JN = 18,    CALL = 19,
        RET = 20,   PUSH = 21,  POP = 22,   IN = 23,    OUT = 24
    };
    
    // Addressing modes from architecture specification
    enum class AddressingMode : uint8_t {
        REGISTER = 0,           // Register mode
        IMMEDIATE = 1,          // Immediate (extra word)
        DIRECT = 2,             // Direct memory (absolute, extra word)
        REGISTER_INDIRECT = 3,  // Register indirect
        REGISTER_OFFSET = 4,    // Register + offset (extra word)
        PC_RELATIVE = 5         // PC-relative (extra word)
    };
    
    // Decoded instruction structure
    struct DecodedInstruction {
        Opcode opcode;
        AddressingMode mode;
        uint8_t rd;         // Destination register (0-3)
        uint8_t rs;         // Source register (0-3)
        uint16_t extra_word; // For immediate/address/offset
        bool has_extra_word;
    };
    
    CPU();
    ~CPU() = default;
    
    // Main execution interface
    void reset();
    void load_program(const std::vector<uint8_t>& program, uint16_t start_address = 0x8000);
    void run();                    // Run until HALT
    bool step();                   // Execute one instruction, return false if HALT
    
    // CPU state access
    const Registers& get_registers() const { return registers_; }
    const Memory& get_memory() const { return memory_; }
    bool is_halted() const { return halted_; }
    
    // Debug interface
    void dump_state() const;
    void set_debug_mode(bool enabled) { debug_mode_ = enabled; }

private:
    // CPU components
    Memory memory_;
    Registers registers_;
    ALU alu_;
    
    // CPU state
    bool halted_;
    bool debug_mode_;
    
    // Fetch-Decode-Execute cycle
    void fetch();
    DecodedInstruction decode();
    void execute(const DecodedInstruction& instr);
    
    // Instruction decoding helpers
    Opcode extract_opcode(uint16_t instruction_word);
    AddressingMode extract_mode(uint16_t instruction_word);
    uint8_t extract_rd(uint16_t instruction_word);
    uint8_t extract_rs(uint16_t instruction_word);
    
    // Addressing mode resolution
    uint16_t resolve_operand(const DecodedInstruction& instr, bool is_destination = false);
    uint16_t calculate_effective_address(const DecodedInstruction& instr);
    
    // Instruction implementations
    void execute_nop();
    void execute_halt();
    void execute_mov(const DecodedInstruction& instr);
    void execute_load(const DecodedInstruction& instr);
    void execute_store(const DecodedInstruction& instr);
    void execute_arithmetic(const DecodedInstruction& instr, ALU::Operation op);
    void execute_jump(const DecodedInstruction& instr);
    void execute_conditional_jump(const DecodedInstruction& instr);
    void execute_call(const DecodedInstruction& instr);
    void execute_ret();
    void execute_push(const DecodedInstruction& instr);
    void execute_pop(const DecodedInstruction& instr);
    void execute_io(const DecodedInstruction& instr, bool is_input);
    
    // Helper functions
    void push_word(uint16_t value);
    uint16_t pop_word();
    bool check_condition(Opcode opcode);
    
    // Debug helpers
    void print_instruction(const DecodedInstruction& instr) const;
    std::string opcode_to_string(Opcode opcode) const;
    std::string mode_to_string(AddressingMode mode) const;
};