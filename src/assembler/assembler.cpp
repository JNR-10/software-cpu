#include "assembler.hpp"

#include <cctype>
#include <cstddef>
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

//   1) Tokenize each source line into simple tokens
//   2) Parse tokens into a Line struct (optional label, op, operands)
//   3) Pass 1: assign word addresses and collect label definitions
//   4) Pass 2: encode instructions/directives into 16-bit words
//   5) Return a vector<uint8_t> with little-endian words

namespace {

// A very small token type for this scratch assembler. We only care about
// identifiers (mnemonics, directives, labels), numbers, registers, and a
// few punctuation tokens.
struct Token {
    enum class Type {
        Identifier,
        Number,
        Register,
        Comma,
        Colon,
        Hash,
    } type;
    std::string text;
};

// Trim leading and trailing whitespace from a string.
std::string trim(const std::string& s) {
    std::size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    std::size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// Case-insensitive string compare (ASCII only), used for mnemonics.
bool iequals(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (std::toupper(static_cast<unsigned char>(a[i])) !=
            std::toupper(static_cast<unsigned char>(b[i]))) {
            return false;
        }
    }
    return true;
}

// Turn one source line into a sequence of tokens.
// Comments starting with ';' are stripped before tokenizing.
std::vector<Token> tokenize_line(const std::string& line) {
    std::vector<Token> tokens;

    std::size_t comment_pos = line.find(';');
    std::string work = (comment_pos == std::string::npos) ? line : line.substr(0, comment_pos);

    std::size_t i = 0;
    while (i < work.size()) {
        char c = work[i];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            ++i;
            continue;
        }
        if (c == ',') {
            tokens.push_back({Token::Type::Comma, ","});
            ++i;
            continue;
        }
        if (c == ':') {
            tokens.push_back({Token::Type::Colon, ":"});
            ++i;
            continue;
        }
        if (c == '#') {
            tokens.push_back({Token::Type::Hash, "#"});
            ++i;
            continue;
        }
        // Numeric literal: decimal or hex (0x...). We keep the raw text
        // and interpret it later in parse_number16.
        if (std::isdigit(static_cast<unsigned char>(c))) {
            std::size_t start = i;
            ++i;
            while (i < work.size() && (std::isalnum(static_cast<unsigned char>(work[i])) || work[i] == 'x' || work[i] == 'X')) {
                ++i;
            }
            tokens.push_back({Token::Type::Number, work.substr(start, i - start)});
            continue;
        }
        // Identifiers / directives (allow leading '.' for directives like .org)
        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_' || c == '.') {
            std::size_t start = i;
            ++i;
            while (i < work.size() && (std::isalnum(static_cast<unsigned char>(work[i])) || work[i] == '_' || work[i] == '.')) {
                ++i;
            }
            std::string ident = work.substr(start, i - start);
            std::string upper;
            upper.reserve(ident.size());
            for (char ch : ident) upper.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(ch))));
            if (upper == "R0" || upper == "R1" || upper == "R2" || upper == "R3") {
                tokens.push_back({Token::Type::Register, upper});
            } else {
                tokens.push_back({Token::Type::Identifier, upper});
            }
            continue;
        }
        throw std::runtime_error("Unexpected character in source line");
    }
    return tokens;
}

// Operand in a parsed line: register, immediate (#num), label reference,
// or plain number (for .word / absolute addresses).
struct Operand {
    enum class Kind { Reg, Imm, Label, Number } kind;
    std::string text;
};

// Parsed representation of a source line after tokenization.
// Example:
//   start: ADD R0, #1
// becomes
//   label = "START", op = "ADD", operands = [Reg R0, Imm 1]
struct Line {
    std::string label;      // empty if none
    std::string op;         // uppercased mnemonic or directive (e.g. "ADD", ".ORG")
    bool is_directive = false;
    std::vector<Operand> operands;
};

// Parse a numeric literal into a 16-bit value. Supports decimal and 0xHEX.
std::uint16_t parse_number16(const std::string& text) {
    int base = 10;
    if (text.size() > 2 && text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) {
        base = 16;
    }
    int val = std::stoi(text, nullptr, base);
    if (val < 0 || val > 0xFFFF) throw std::runtime_error("number out of range");
    return static_cast<std::uint16_t>(val);
}

// Map register name (R0..R3) to its numeric index.
std::uint8_t reg_id_from_name(const std::string& name) {
    if (iequals(name, "R0")) return 0;
    if (iequals(name, "R1")) return 1;
    if (iequals(name, "R2")) return 2;
    if (iequals(name, "R3")) return 3;
    throw std::runtime_error("Unknown register: " + name);
}

// Convert a token stream for one line into a Line struct.
// Handles an optional leading label (IDENT ':'), then an opcode/directive
// and a comma-separated operand list.
Line parse_line_tokens(const std::vector<Token>& tokens) {
    Line line;
    std::size_t idx = 0;
    if (tokens.empty()) return line;

    // label?
    if (idx + 1 < tokens.size() && tokens[idx].type == Token::Type::Identifier && tokens[idx + 1].type == Token::Type::Colon) {
        line.label = tokens[idx].text;
        idx += 2;
    }
    if (idx >= tokens.size()) return line;

    if (tokens[idx].type != Token::Type::Identifier) {
        throw std::runtime_error("Expected mnemonic or directive");
    }
    line.op = tokens[idx].text;
    line.is_directive = !line.op.empty() && line.op[0] == '.';
    ++idx;

    while (idx < tokens.size()) {
        if (tokens[idx].type == Token::Type::Comma) {
            ++idx;
            continue;
        }
        Operand opnd;
        if (tokens[idx].type == Token::Type::Register) {
            opnd.kind = Operand::Kind::Reg;
            opnd.text = tokens[idx].text;
            ++idx;
        } else if (tokens[idx].type == Token::Type::Hash) {
            ++idx;
            if (idx >= tokens.size() || tokens[idx].type != Token::Type::Number) {
                throw std::runtime_error("Expected number after '#'");
            }
            opnd.kind = Operand::Kind::Imm;
            opnd.text = tokens[idx].text;
            ++idx;
        } else if (tokens[idx].type == Token::Type::Number) {
            opnd.kind = Operand::Kind::Number;
            opnd.text = tokens[idx].text;
            ++idx;
        } else if (tokens[idx].type == Token::Type::Identifier) {
            opnd.kind = Operand::Kind::Label;
            opnd.text = tokens[idx].text;
            ++idx;
        } else {
            throw std::runtime_error("Unsupported operand token");
        }
        line.operands.push_back(opnd);
    }

    return line;
}

// Construct the 16-bit instruction word given opcode/mode/rd/rs using
// the Phase 1 base instruction format:
//   15..11 opcode, 10..8 mode, 7..5 RD, 4..2 RS, 1..0 unused
std::uint16_t make_instr_word(std::uint8_t opcode, std::uint8_t mode,
                              std::uint8_t rd, std::uint8_t rs) {
    std::uint16_t w = 0;
    w |= static_cast<std::uint16_t>(opcode & 0x1F) << 11;
    w |= static_cast<std::uint16_t>(mode   & 0x07) << 8;
    w |= static_cast<std::uint16_t>(rd     & 0x07) << 5;
    w |= static_cast<std::uint16_t>(rs     & 0x07) << 2;
    return w;
}

} // namespace

std::vector<std::uint8_t> assemble(const std::string& source) {
    std::istringstream iss(source);
    std::string raw_line;

    std::vector<Line> lines;
    while (std::getline(iss, raw_line)) {
        std::string t = trim(raw_line);
        if (t.empty()) continue;
        auto tokens = tokenize_line(t);
        lines.push_back(parse_line_tokens(tokens));
    }

    // Pass 1: symbol table and addresses (word addresses)
    std::unordered_map<std::string, std::uint16_t> symbols;
    std::vector<std::uint16_t> line_addr(lines.size());
    std::uint16_t addr = 0x8000; // default org

    for (std::size_t i = 0; i < lines.size(); ++i) {
        line_addr[i] = addr;
        Line& l = lines[i];
        if (!l.label.empty()) {
            if (symbols.count(l.label)) throw std::runtime_error("Duplicate label: " + l.label);
            symbols[l.label] = addr;
        }
        // If line has only a label and no op, it doesn't emit code
        if (l.op.empty()) {
            continue;
        }

        if (l.is_directive) {
            if (iequals(l.op, ".ORG")) {
                if (l.operands.size() != 1 || (l.operands[0].kind != Operand::Kind::Number && l.operands[0].kind != Operand::Kind::Label)) {
                    throw std::runtime_error(".org expects one numeric or label operand");
                }
                std::uint16_t v;
                if (l.operands[0].kind == Operand::Kind::Number) v = parse_number16(l.operands[0].text);
                else v = symbols.at(l.operands[0].text);
                addr = v;
            } else if (iequals(l.op, ".WORD")) {
                addr = static_cast<std::uint16_t>(addr + 1);
            }
        } else {
            // instructions we support: NOP, HALT, ADD, JMP, JZ
            if (iequals(l.op, "NOP") || iequals(l.op, "HALT")) {
                addr = static_cast<std::uint16_t>(addr + 1);
            } else if (iequals(l.op, "ADD")) {
                addr = static_cast<std::uint16_t>(addr + 1);
                if (l.operands.size() == 2 && l.operands[1].kind == Operand::Kind::Imm) {
                    addr = static_cast<std::uint16_t>(addr + 1);
                }
            } else if (iequals(l.op, "JMP") || iequals(l.op, "JZ")) {
                addr = static_cast<std::uint16_t>(addr + 2); // instr + offset
            }
        }
    }

    // Pass 2: encode
    std::vector<std::uint16_t> words;
    for (std::size_t i = 0; i < lines.size(); ++i) {
        const Line& l = lines[i];
        std::uint16_t cur_addr = line_addr[i];
        if (l.op.empty()) {
            continue;
        }

        if (l.is_directive) {
            if (iequals(l.op, ".ORG")) continue;
            if (iequals(l.op, ".WORD")) {
                if (l.operands.size() != 1) throw std::runtime_error(".word expects one operand");
                std::uint16_t v;
                if (l.operands[0].kind == Operand::Kind::Number) v = parse_number16(l.operands[0].text);
                else if (l.operands[0].kind == Operand::Kind::Label) v = symbols.at(l.operands[0].text);
                else throw std::runtime_error("Unsupported .word operand");
                words.push_back(v);
            }
            continue;
        }

        std::uint8_t opcode = 0;
        if      (iequals(l.op, "NOP"))  opcode = 0;
        else if (iequals(l.op, "HALT")) opcode = 1;
        else if (iequals(l.op, "ADD"))  opcode = 5;
        else if (iequals(l.op, "JMP"))  opcode = 13;
        else if (iequals(l.op, "JZ"))   opcode = 14;
        else throw std::runtime_error("Unsupported instruction: " + l.op);

        std::uint8_t mode = 0;
        std::uint8_t rd = 0;
        std::uint8_t rs = 0;

        if (opcode == 0 || opcode == 1) {
            // NOP / HALT
            words.push_back(make_instr_word(opcode, 0, 0, 0));
        } else if (opcode == 5) {
            // ADD
            if (l.operands.size() != 2) throw std::runtime_error("ADD expects two operands");
            if (l.operands[0].kind != Operand::Kind::Reg) throw std::runtime_error("ADD first operand must be reg");
            rd = reg_id_from_name(l.operands[0].text);
            if (l.operands[1].kind == Operand::Kind::Reg) {
                mode = 0;
                rs = reg_id_from_name(l.operands[1].text);
                words.push_back(make_instr_word(opcode, mode, rd, rs));
            } else if (l.operands[1].kind == Operand::Kind::Imm) {
                mode = 1;
                std::uint16_t imm = parse_number16(l.operands[1].text);
                words.push_back(make_instr_word(opcode, mode, rd, 0));
                words.push_back(imm);
            } else {
                throw std::runtime_error("Unsupported ADD second operand");
            }
        } else if (opcode == 13 || opcode == 14) {
            // JMP / JZ, PC-relative
            if (l.operands.size() != 1) throw std::runtime_error("Jump expects one operand");
            mode = 5;
            std::uint16_t target;
            if (l.operands[0].kind == Operand::Kind::Label) {
                auto it = symbols.find(l.operands[0].text);
                if (it == symbols.end()) throw std::runtime_error("Unknown label in jump: " + l.operands[0].text);
                target = it->second;
            } else if (l.operands[0].kind == Operand::Kind::Number) {
                target = parse_number16(l.operands[0].text);
            } else {
                throw std::runtime_error("Unsupported jump operand");
            }
            std::int32_t next_pc = static_cast<std::int32_t>(cur_addr) + 2; // instr + offset
            std::int32_t offset = static_cast<std::int32_t>(target) - next_pc;
            std::uint16_t off16 = static_cast<std::uint16_t>(offset & 0xFFFF);
            words.push_back(make_instr_word(opcode, mode, 0, 0));
            words.push_back(off16);
        }
    }

    // Convert words to bytes (little-endian)
    std::vector<std::uint8_t> bytes;
    bytes.reserve(words.size() * 2);
    for (std::uint16_t w : words) {
        bytes.push_back(static_cast<std::uint8_t>(w & 0xFF));
        bytes.push_back(static_cast<std::uint8_t>((w >> 8) & 0xFF));
    }

    return bytes;
}
