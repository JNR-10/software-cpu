#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "assembler/assembler.hpp"
#include "emulator/cpu.hpp"

void print_usage(const char* program_name) {
    std::cout << "Usage:" << std::endl;
    std::cout << "  " << program_name << " assemble <input.asm> <output.bin>" << std::endl;
    std::cout << "  " << program_name << " run <program.bin>" << std::endl;
    std::cout << "  " << program_name << " test" << std::endl;
}

int assemble_file(const std::string& input_path, const std::string& output_path) {
    std::ifstream in(input_path);
    if (!in) {
        std::cerr << "Failed to open input file: " << input_path << "\n";
        return 1;
    }

    std::string source((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

    std::vector<std::uint8_t> bytes;
    try {
        bytes = assemble(source);
    } catch (const std::exception& ex) {
        std::cerr << "Assembly error: " << ex.what() << "\n";
        return 1;
    }

    std::ofstream out(output_path, std::ios::binary);
    if (!out) {
        std::cerr << "Failed to open output file: " << output_path << "\n";
        return 1;
    }

    out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    std::cout << "Assembled " << bytes.size() << " bytes to " << output_path << "\n";
    return 0;
}

int run_program(const std::string& program_path) {
    std::ifstream in(program_path, std::ios::binary);
    if (!in) {
        std::cerr << "Failed to open program file: " << program_path << "\n";
        return 1;
    }

    std::vector<uint8_t> program((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    
    std::cout << "Loading program (" << program.size() << " bytes)..." << std::endl;
    
    CPU cpu;
    cpu.set_debug_mode(true);
    cpu.load_program(program);
    
    std::cout << "Running program..." << std::endl;
    cpu.run();
    
    std::cout << "Program execution complete." << std::endl;
    cpu.dump_state();
    
    return 0;
}

int run_test() {
    std::cout << "Running emulator test..." << std::endl;
    
    // Simple hardcoded test program
    CPU cpu;
    cpu.set_debug_mode(true);
    
    // Create test program: ADD R0, #42; HALT
    // Correct instruction encoding:
    // ADD R0, #42: opcode=5, mode=1, rd=0, rs=0
    // Instruction word: (5 << 11) | (1 << 8) | (0 << 5) | (0 << 2) = 0x2900
    std::vector<uint8_t> program = {
        0x00, 0x29,  // ADD R0, #42 (0x2900 in little-endian)
        0x2A, 0x00,  // Immediate value: 42
        0x00, 0x08   // HALT (0x0800 in little-endian)
    };
    
    cpu.load_program(program);
    cpu.run();
    
    const Registers& regs = cpu.get_registers();
    if (regs.get_gpr(0) == 42) {
        std::cout << "✅ Test passed!" << std::endl;
        return 0;
    } else {
        std::cout << "❌ Test failed! R0 = " << regs.get_gpr(0) << std::endl;
        return 1;
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    std::string command = argv[1];
    
    if (command == "assemble" && argc == 4) {
        return assemble_file(argv[2], argv[3]);
    } else if (command == "run" && argc == 3) {
        return run_program(argv[2]);
    } else if (command == "test" && argc == 2) {
        return run_test();
    } else {
        print_usage(argv[0]);
        return 1;
    }
}