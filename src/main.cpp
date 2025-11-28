#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "assembler/assembler.hpp"
int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input.asm> <output.bin>\n";
        return 1;
    }

    const std::string input_path = argv[1];
    const std::string output_path = argv[2];

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
    std::cout << "Wrote " << bytes.size() << " bytes to " << output_path << "\n";
    return 0;
}
