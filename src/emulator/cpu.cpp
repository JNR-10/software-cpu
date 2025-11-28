#include "cpu.hpp"
#include <iostream>
#include <iomanip>
#include <stdexcept>

CPU::CPU() : halted_(false), debug_mode_(false) {
    reset();
}

void CPU::reset() {
    registers_.reset();
    halted_ = false;
    
    if (debug_mode_) {
        std::cout << "CPU Reset" << std::endl;
    }
}

void CPU::load_program(const std::vector<uint8_t>& program, uint16_t start_address) {
    memory_.load_program(program, start_address);
    registers_.set_pc(start_address);
    
    if (debug_mode_) {
        std::cout << "Program loaded at 0x" << std::hex << start_address 
                  << ", size: " << std::dec << program.size() << " bytes" << std::endl;
    }
}

void CPU::run() {
    if (debug_mode_) {
        std::cout << "Starting CPU execution..." << std::endl;
    }
    
    while (!halted_ && step()) {
        // Continue execution
    }
    
    if (debug_mode_) {
        std::cout << "CPU execution stopped. Halted: " << (halted_ ? "Yes" : "No") << std::endl;
    }
}

bool CPU::step() {
    if (halted_) return false;
    
    try {
        // Fetch-Decode-Execute cycle
        fetch();
        DecodedInstruction instr = decode();
        
        if (debug_mode_) {
            print_instruction(instr);
        }
        
        execute(instr);
        
        return !halted_;
    } catch (const std::exception& e) {
        std::cerr << "CPU Error: " << e.what() << std::endl;
        halted_ = true;
        return false;
    }
}

void CPU::fetch() {
    // Fetch phase: MAR ← PC, MDR ← MEM[MAR], IR ← MDR, PC ← PC + 2
    registers_.set_mar(registers_.get_pc());
    registers_.set_mdr(memory_.read_word(registers_.get_mar()));
    registers_.set_ir(registers_.get_mdr());
    registers_.increment_pc(2);
}

CPU::DecodedInstruction CPU::decode() {
    DecodedInstruction instr;
    uint16_t ir = registers_.get_ir();
    
    // Extract fields from instruction word
    instr.opcode = extract_opcode(ir);
    instr.mode = extract_mode(ir);
    instr.rd = extract_rd(ir);
    instr.rs = extract_rs(ir);
    instr.has_extra_word = false;
    instr.extra_word = 0;
    
    // Check if instruction needs extra word
    if (instr.mode == AddressingMode::IMMEDIATE ||
        instr.mode == AddressingMode::DIRECT ||
        instr.mode == AddressingMode::REGISTER_OFFSET ||
        instr.mode == AddressingMode::PC_RELATIVE) {
        
        instr.has_extra_word = true;
        instr.extra_word = memory_.read_word(registers_.get_pc());
        registers_.increment_pc(2);
    }
    
    return instr;
}

void CPU::execute(const DecodedInstruction& instr) {
    switch (instr.opcode) {
        case Opcode::NOP:
            execute_nop();
            break;
        case Opcode::HALT:
            execute_halt();
            break;
        case Opcode::MOV:
            execute_mov(instr);
            break;
        case Opcode::LOAD:
            execute_load(instr);
            break;
        case Opcode::STORE:
            execute_store(instr);
            break;
        case Opcode::ADD:
            execute_arithmetic(instr, ALU::Operation::ADD);
            break;
        case Opcode::SUB:
            execute_arithmetic(instr, ALU::Operation::SUB);
            break;
        case Opcode::AND:
            execute_arithmetic(instr, ALU::Operation::AND);
            break;
        case Opcode::OR:
            execute_arithmetic(instr, ALU::Operation::OR);
            break;
        case Opcode::XOR:
            execute_arithmetic(instr, ALU::Operation::XOR);
            break;
        case Opcode::CMP:
            execute_arithmetic(instr, ALU::Operation::CMP);
            break;
        case Opcode::SHL:
            execute_arithmetic(instr, ALU::Operation::SHL);
            break;
        case Opcode::SHR:
            execute_arithmetic(instr, ALU::Operation::SHR);
            break;
        case Opcode::JMP:
            execute_jump(instr);
            break;
        case Opcode::JZ:
        case Opcode::JNZ:
        case Opcode::JC:
        case Opcode::JNC:
        case Opcode::JN:
            execute_conditional_jump(instr);
            break;
        case Opcode::CALL:
            execute_call(instr);
            break;
        case Opcode::RET:
            execute_ret();
            break;
        case Opcode::PUSH:
            execute_push(instr);
            break;
        case Opcode::POP:
            execute_pop(instr);
            break;
        case Opcode::IN:
            execute_io(instr, true);
            break;
        case Opcode::OUT:
            execute_io(instr, false);
            break;
        default:
            throw std::runtime_error("Unknown opcode: " + std::to_string(static_cast<int>(instr.opcode)));
    }
}

// Instruction field extraction
CPU::Opcode CPU::extract_opcode(uint16_t instruction_word) {
    return static_cast<Opcode>((instruction_word >> 11) & 0x1F);
}

CPU::AddressingMode CPU::extract_mode(uint16_t instruction_word) {
    return static_cast<AddressingMode>((instruction_word >> 8) & 0x07);
}

uint8_t CPU::extract_rd(uint16_t instruction_word) {
    return static_cast<uint8_t>((instruction_word >> 5) & 0x07);
}

uint8_t CPU::extract_rs(uint16_t instruction_word) {
    return static_cast<uint8_t>((instruction_word >> 2) & 0x07);
}

// Simple instruction implementations
void CPU::execute_nop() {
    // Do nothing
}

void CPU::execute_halt() {
    halted_ = true;
    if (debug_mode_) {
        std::cout << "CPU HALTED" << std::endl;
    }
}

void CPU::execute_arithmetic(const DecodedInstruction& instr, ALU::Operation op) {
    uint16_t operand_a = registers_.get_gpr(instr.rd);
    uint16_t operand_b = resolve_operand(instr);
    
    uint16_t result = alu_.execute(op, operand_a, operand_b, registers_);
    
    // For CMP, don't store result
    if (op != ALU::Operation::CMP) {
        registers_.set_gpr(instr.rd, result);
    }
}

uint16_t CPU::resolve_operand(const DecodedInstruction& instr, bool is_destination) {
    switch (instr.mode) {
        case AddressingMode::REGISTER:
            return registers_.get_gpr(instr.rs);
            
        case AddressingMode::IMMEDIATE:
            if (is_destination) {
                throw std::runtime_error("Cannot use immediate mode for destination");
            }
            return instr.extra_word;
            
        case AddressingMode::DIRECT:
            if (is_destination) {
                memory_.write_word(instr.extra_word, registers_.get_gpr(instr.rd));
                return 0; // Not used for destination writes
            }
            return memory_.read_word(instr.extra_word);
            
        case AddressingMode::REGISTER_INDIRECT: {
            uint16_t address = registers_.get_gpr(instr.rs);
            if (is_destination) {
                memory_.write_word(address, registers_.get_gpr(instr.rd));
                return 0;
            }
            return memory_.read_word(address);
        }
        
        default:
            throw std::runtime_error("Unsupported addressing mode");
    }
}

void CPU::dump_state() const {
    std::cout << "\n=== CPU State ===" << std::endl;
    registers_.dump_registers();
    std::cout << "Halted: " << (halted_ ? "Yes" : "No") << std::endl;
    std::cout << "=================" << std::endl;
}

void CPU::print_instruction(const DecodedInstruction& instr) const {
    std::cout << "PC: 0x" << std::hex << std::setw(4) << std::setfill('0') 
              << (registers_.get_pc() - (instr.has_extra_word ? 4 : 2))
              << " | " << opcode_to_string(instr.opcode)
              << " (mode: " << mode_to_string(instr.mode) << ")"
              << std::dec << std::endl;
}

std::string CPU::opcode_to_string(Opcode opcode) const {
    switch (opcode) {
        case Opcode::NOP: return "NOP";
        case Opcode::HALT: return "HALT";
        case Opcode::ADD: return "ADD";
        case Opcode::SUB: return "SUB";
        case Opcode::JMP: return "JMP";
        case Opcode::JZ: return "JZ";
        default: return "UNKNOWN";
    }
}

std::string CPU::mode_to_string(AddressingMode mode) const {
    switch (mode) {
        case AddressingMode::REGISTER: return "REG";
        case AddressingMode::IMMEDIATE: return "IMM";
        case AddressingMode::DIRECT: return "DIR";
        case AddressingMode::REGISTER_INDIRECT: return "IND";
        case AddressingMode::PC_RELATIVE: return "REL";
        default: return "UNK";
    }
}

void CPU::execute_mov(const DecodedInstruction& instr) {
    // Simple MOV implementation - copy from source to destination
    uint16_t value = resolve_operand(instr);
    registers_.set_gpr(instr.rd, value);
}

void CPU::execute_load(const DecodedInstruction& instr) {
    // LOAD: Read from memory into register
    uint16_t address = calculate_effective_address(instr);
    uint16_t value = memory_.read_word(address);
    registers_.set_gpr(instr.rd, value);
}

void CPU::execute_store(const DecodedInstruction& instr) {
    // STORE: Write register to memory
    uint16_t address = calculate_effective_address(instr);
    uint16_t value = registers_.get_gpr(instr.rd);
    memory_.write_word(address, value);
}

void CPU::execute_jump(const DecodedInstruction& instr) {
    // Unconditional jump
    uint16_t target = calculate_effective_address(instr);
    registers_.set_pc(target);
}

void CPU::execute_conditional_jump(const DecodedInstruction& instr) {
    // Conditional jump based on flags
    if (check_condition(instr.opcode)) {
        uint16_t target = calculate_effective_address(instr);
        registers_.set_pc(target);
    }
}

void CPU::execute_call(const DecodedInstruction& instr) {
    // CALL: Push PC and jump
    push_word(registers_.get_pc());
    uint16_t target = calculate_effective_address(instr);
    registers_.set_pc(target);
}

void CPU::execute_ret() {
    // RET: Pop PC from stack
    uint16_t return_address = pop_word();
    registers_.set_pc(return_address);
}

void CPU::execute_push(const DecodedInstruction& instr) {
    // PUSH: Push register onto stack
    uint16_t value = registers_.get_gpr(instr.rd);
    push_word(value);
}

void CPU::execute_pop(const DecodedInstruction& instr) {
    // POP: Pop from stack into register
    uint16_t value = pop_word();
    registers_.set_gpr(instr.rd, value);
}

void CPU::execute_io(const DecodedInstruction& instr, bool is_input) {
    if (is_input) {
        // IN: Read from I/O port
        uint16_t port = resolve_operand(instr);
        uint8_t value = memory_.read_byte(Memory::IO_START + (port & 0xFF));
        registers_.set_gpr(instr.rd, static_cast<uint16_t>(value));
    } else {
        // OUT: Write to I/O port
        uint16_t port = resolve_operand(instr);
        uint8_t value = static_cast<uint8_t>(registers_.get_gpr(instr.rd) & 0xFF);
        memory_.write_byte(Memory::IO_START + (port & 0xFF), value);
    }
}

// Helper functions
uint16_t CPU::calculate_effective_address(const DecodedInstruction& instr) {
    switch (instr.mode) {
        case AddressingMode::DIRECT:
            return instr.extra_word;
        case AddressingMode::REGISTER_INDIRECT:
            return registers_.get_gpr(instr.rs);
        case AddressingMode::REGISTER_OFFSET:
            return registers_.get_gpr(instr.rs) + instr.extra_word;
        case AddressingMode::PC_RELATIVE:
            return registers_.get_pc() + static_cast<int16_t>(instr.extra_word);
        default:
            throw std::runtime_error("Invalid addressing mode for address calculation");
    }
}

void CPU::push_word(uint16_t value) {
    registers_.push_sp();  // Pre-decrement SP
    memory_.write_word(registers_.get_sp(), value);
}

uint16_t CPU::pop_word() {
    uint16_t value = memory_.read_word(registers_.get_sp());
    registers_.pop_sp();   // Post-increment SP
    return value;
}

bool CPU::check_condition(Opcode opcode) {
    switch (opcode) {
        case Opcode::JZ:  return registers_.is_zero();
        case Opcode::JNZ: return !registers_.is_zero();
        case Opcode::JC:  return registers_.is_carry();
        case Opcode::JNC: return !registers_.is_carry();
        case Opcode::JN:  return registers_.is_negative();
        default: return false;
    }
}