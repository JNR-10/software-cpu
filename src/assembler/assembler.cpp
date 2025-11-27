 ﻿#include "assembler.hpp"
#include <cctype>
#include <cstdint>
#include <cstddef>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

// ---------------- Tokenization ----------------

enum class TokenType {
    Identifier,
    Number,
    Register,
    Comma,
    Colon,
    Hash,
    LBracket,
    RBracket,
    Plus,
};

struct Token {
    TokenType type;
    std::string text;
    int line;
    int column;
};

std::string trim(const std::string& s) {
    std::size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    std::size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

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

std::vector<Token> tokenize_line(const std::string& line, int line_no) {
    std::vector<Token> tokens;

    // Strip comments starting with ';'
    std::size_t comment_pos = line.find(';');
    std::string work = (comment_pos == std::string::npos) ? line : line.substr(0, comment_pos);

    int i = 0;
    int n = static_cast<int>(work.size());
    while (i < n) {
        char c = work[i];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            ++i;
            continue;
        }

        if (c == ',') {
            tokens.push_back({TokenType::Comma, std::string(1, c), line_no, i});
            ++i;
            continue;
        }
        if (c == ':') {
            tokens.push_back({TokenType::Colon, std::string(1, c), line_no, i});
            ++i;
            continue;
        }
        if (c == '#') {
            tokens.push_back({TokenType::Hash, std::string(1, c), line_no, i});
            ++i;
            continue;
        }
        if (c == '[') {
            tokens.push_back({TokenType::LBracket, std::string(1, c), line_no, i});
            ++i;
            continue;
        }
        if (c == ']') {
            tokens.push_back({TokenType::RBracket, std::string(1, c), line_no, i});
            ++i;
            continue;
        }
        if (c == '+') {
            tokens.push_back({TokenType::Plus, std::string(1, c), line_no, i});
            ++i;
            continue;
        }

        if (std::isdigit(static_cast<unsigned char>(c))) {
            int start = i;
            ++i;
            while (i < n && (std::isalnum(static_cast<unsigned char>(work[i])) || work[i] == 'x' || work[i] == 'X')) {
                ++i;
            }
            tokens.push_back({TokenType::Number, work.substr(start, i - start), line_no, start});
            continue;
        }

        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            int start = i;
            ++i;
            while (i < n && (std::isalnum(static_cast<unsigned char>(work[i])) || work[i] == '_')) {
                ++i;
            }
            std::string ident = work.substr(start, i - start);
            std::string upper;
            upper.reserve(ident.size());
            for (char ch : ident) {
                upper.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(ch))));
            }
            if (upper == "R0" || upper == "R1" || upper == "R2" || upper == "R3") {
                tokens.push_back({TokenType::Register, upper, line_no, start});
            } else {
                tokens.push_back({TokenType::Identifier, ident, line_no, start});
            }
            continue;
        }

        throw std::runtime_error("Unexpected character in source");
    }

    return tokens;
}

// ---------------- Parsing ----------------

enum class LineKind {
    Empty,
    Instruction,
    Directive,
};

enum class OperandKind {
    Reg,
    Imm,
    LabelRef,
    RawNumber,
};

struct Operand {
    OperandKind kind;
    std::string text;
};

struct ParsedLine {
    int line_no = 0;
    std::string label;
    LineKind kind = LineKind::Empty;
    std::string op;
    std::vector<Operand> operands;
};

ParsedLine parse_tokens(const std::vector<Token>& tokens) {
    ParsedLine pl;
    if (tokens.empty()) {
        pl.kind = LineKind::Empty;
        return pl;
    }

    pl.line_no = tokens.front().line;
    std::size_t idx = 0;

    if (idx + 1 < tokens.size() && tokens[idx].type == TokenType::Identifier && tokens[idx + 1].type == TokenType::Colon) {
        pl.label = tokens[idx].text;
        idx += 2;
    }

    if (idx >= tokens.size()) {
        pl.kind = LineKind::Empty;
        return pl;
    }

    const Token& t = tokens[idx];
    if (t.type != TokenType::Identifier) {
        throw std::runtime_error("Expected instruction or directive");
    }

    pl.op = t.text;
    bool is_directive = !pl.op.empty() && pl.op[0] == '.';
    pl.kind = is_directive ? LineKind::Directive : LineKind::Instruction;
    ++idx;

    while (idx < tokens.size()) {
        if (tokens[idx].type == TokenType::Comma) {
            ++idx;
            continue;
        }

        Operand operand;

        if (tokens[idx].type == TokenType::Register) {
            operand.kind = OperandKind::Reg;
            operand.text = tokens[idx].text;
            ++idx;
        } else if (tokens[idx].type == TokenType::Hash) {
            ++idx;
            if (idx >= tokens.size() || tokens[idx].type != TokenType::Number) {
                throw std::runtime_error("Expected number after '#'");
            }
            operand.kind = OperandKind::Imm;
            operand.text = tokens[idx].text;
            ++idx;
        } else if (tokens[idx].type == TokenType::Number) {
            operand.kind = OperandKind::RawNumber;
            operand.text = tokens[idx].text;
            ++idx;
        } else if (tokens[idx].type == TokenType::Identifier) {
            operand.kind = OperandKind::LabelRef;
            operand.text = tokens[idx].text;
            ++idx;
        } else {
            throw std::runtime_error("Unsupported operand syntax");
        }

        pl.operands.push_back(std::move(operand));
    }

    return pl;
}

std::uint16_t parse_number16(const std::string& text) {
    int base = 10;
    std::size_t idx = 0;
    if (text.size() > 2 && text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) {
        base = 16;
    }
    int val = std::stoi(text, &idx, base);
    if (val < 0 || val > 0xFFFF) {
        throw std::runtime_error("Immediate/address out of range for 16-bit value");
    }
    return static_cast<std::uint16_t>(val);
}

std::uint8_t reg_id_from_name(const std::string& name) {
    if (iequals(name, "R0")) return 0;
    if (iequals(name, "R1")) return 1;
    if (iequals(name, "R2")) return 2;
    if (iequals(name, "R3")) return 3;
    throw std::runtime_error("Unknown register");
}

struct AsmContext {
    std::uint16_t current_addr_words = 0x8000;
    std::vector<std::pair<std::string, std::uint16_t>> symbols;
    std::vector<std::uint16_t> line_addr;
};

struct InstrInfo {
    const char* name;
    std::uint8_t opcode;
};

const InstrInfo INSTR_TABLE[] = {
    {"NOP",  0},
    {"HALT", 1},
    {"ADD",  5},
    {"JMP",  13},
    {"JZ",   14},
};

const InstrInfo* find_instr(const std::string& op_upper) {
    for (const auto& info : INSTR_TABLE) {
        if (iequals(op_upper, info.name)) {
            return &info;
        }
    }
    return nullptr;
}

bool has_instr(const std::string& op_upper) {
    return find_instr(op_upper) != nullptr;
}

bool symbol_exists(const AsmContext& ctx, const std::string& name) {
    for (const auto& p : ctx.symbols) {
        if (p.first == name) return true;
    }
    return false;
}

std::uint16_t lookup_symbol(const AsmContext& ctx, const std::string& name) {
    for (const auto& p : ctx.symbols) {
        if (p.first == name) return p.second;
    }
    throw std::runtime_error("Unknown label: " + name);
}

void pass1(const std::vector<ParsedLine>& lines, AsmContext& ctx) {
    ctx.line_addr.resize(lines.size());

    for (std::size_t i = 0; i < lines.size(); ++i) {
        const ParsedLine& pl = lines[i];
        ctx.line_addr[i] = ctx.current_addr_words;

        if (!pl.label.empty()) {
            if (symbol_exists(ctx, pl.label)) {
                throw std::runtime_error("Duplicate label");
            }
            ctx.symbols.emplace_back(pl.label, ctx.current_addr_words);
        }

        if (pl.kind == LineKind::Directive) {
            if (iequals(pl.op, ".org")) {
                if (pl.operands.size() != 1 || pl.operands[0].kind != OperandKind::RawNumber) {
                    throw std::runtime_error(".org expects one numeric operand");
                }
                ctx.current_addr_words = parse_number16(pl.operands[0].text);
            } else if (iequals(pl.op, ".word")) {
                ctx.current_addr_words = static_cast<std::uint16_t>(ctx.current_addr_words + 1);
            }
        } else if (pl.kind == LineKind::Instruction) {
            std::string upper;
            upper.reserve(pl.op.size());
            for (char ch : pl.op) {
                upper.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(ch))));
            }

            if (!has_instr(upper)) {
                continue;
            }

            ctx.current_addr_words = static_cast<std::uint16_t>(ctx.current_addr_words + 1);

            if (iequals(upper, "ADD")) {
                if (pl.operands.size() == 2 && pl.operands[1].kind == OperandKind::Imm) {
                    ctx.current_addr_words = static_cast<std::uint16_t>(ctx.current_addr_words + 1);
                }
            } else if (iequals(upper, "JMP") || iequals(upper, "JZ")) {
                ctx.current_addr_words = static_cast<std::uint16_t>(ctx.current_addr_words + 1);
            }
        }
    }
}

// ---------------- Pass 2 ----------------

std::uint16_t make_instr_word(std::uint8_t opcode, std::uint8_t mode,
                              std::uint8_t rd, std::uint8_t rs) {
    std::uint16_t w = 0;
    w |= static_cast<std::uint16_t>(opcode & 0x1F) << 11;
    w |= static_cast<std::uint16_t>(mode   & 0x07) << 8;
    w |= static_cast<std::uint16_t>(rd     & 0x07) << 5;
    w |= static_cast<std::uint16_t>(rs     & 0x07) << 2;
    return w;
}

void pass2(const std::vector<ParsedLine>& lines,
           const AsmContext& ctx,
           std::vector<std::uint16_t>& words) {
    std::uint16_t current_addr_words = 0x8000;

    for (std::size_t i = 0; i < lines.size(); ++i) {
        const ParsedLine& pl = lines[i];
        current_addr_words = ctx.line_addr[i];

        if (pl.kind == LineKind::Directive) {
            if (iequals(pl.op, ".org")) {
                continue;
            } else if (iequals(pl.op, ".word")) {
                if (pl.operands.size() != 1) {
                    throw std::runtime_error(".word expects one operand");
                }
                std::uint16_t value;
                if (pl.operands[0].kind == OperandKind::RawNumber) {
                    value = parse_number16(pl.operands[0].text);
                } else if (pl.operands[0].kind == OperandKind::LabelRef) {
                    value = lookup_symbol(ctx, pl.operands[0].text);
                } else {
                    throw std::runtime_error("Unsupported operand for .word");
                }
                words.push_back(value);
            }
            continue;
        }

        if (pl.kind != LineKind::Instruction) {
            continue;
        }

        std::string upper;
        upper.reserve(pl.op.size());
        for (char ch : pl.op) {
            upper.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(ch))));
        }

        const InstrInfo* info = find_instr(upper);
        if (!info) {
            throw std::runtime_error("Unknown instruction");
        }

        std::uint8_t opcode = info->opcode;
        std::uint8_t mode = 0;
        std::uint8_t rd = 0;
        std::uint8_t rs = 0;

        if (iequals(upper, "NOP") || iequals(upper, "HALT")) {
            mode = 0;
        } else if (iequals(upper, "ADD")) {
            if (pl.operands.size() != 2) {
                throw std::runtime_error("ADD expects two operands");
            }
            if (pl.operands[0].kind != OperandKind::Reg) {
                throw std::runtime_error("ADD first operand must be register");
            }
            rd = reg_id_from_name(pl.operands[0].text);

            if (pl.operands[1].kind == OperandKind::Reg) {
                mode = 0;
                rs = reg_id_from_name(pl.operands[1].text);
                std::uint16_t w = make_instr_word(opcode, mode, rd, rs);
                words.push_back(w);
            } else if (pl.operands[1].kind == OperandKind::Imm) {
                mode = 1;
                std::uint16_t imm = parse_number16(pl.operands[1].text);
                std::uint16_t w = make_instr_word(opcode, mode, rd, 0);
                words.push_back(w);
                words.push_back(imm);
            } else {
                throw std::runtime_error("Unsupported ADD operand");
            }
            continue;
        } else if (iequals(upper, "JMP") || iequals(upper, "JZ")) {
            if (pl.operands.size() != 1) {
                throw std::runtime_error("Jump expects one operand");
            }
            mode = 5;
            std::int32_t target_words = 0;
            if (pl.operands[0].kind == OperandKind::LabelRef) {
                target_words = lookup_symbol(ctx, pl.operands[0].text);
            } else if (pl.operands[0].kind == OperandKind::RawNumber) {
                target_words = parse_number16(pl.operands[0].text);
            } else {
                throw std::runtime_error("Unsupported jump operand");
            }

            std::int32_t next_pc_words = static_cast<std::int32_t>(current_addr_words) + 2;
            std::int32_t offset = target_words - next_pc_words;
            std::uint16_t offset16 = static_cast<std::uint16_t>(offset & 0xFFFF);

            std::uint16_t w = make_instr_word(opcode, mode, 0, 0);
            words.push_back(w);
            words.push_back(offset16);
            continue;
        } else {
            throw std::runtime_error("Instruction not yet supported in assembler");
        }

        std::uint16_t w = make_instr_word(opcode, mode, rd, rs);
        words.push_back(w);
    }
}

} // namespace

std::vector<std::uint8_t> assemble(const std::string& source) {
    std::istringstream iss(source);
    std::string line;
    std::vector<ParsedLine> lines;

    int line_no = 1;
    while (std::getline(iss, line)) {
        std::string t = trim(line);
        if (t.empty()) {
            ++line_no;
            continue;
        }
        auto tokens = tokenize_line(line, line_no);
        ParsedLine pl = parse_tokens(tokens);
        lines.push_back(std::move(pl));
        ++line_no;
    }

    AsmContext ctx;
    pass1(lines, ctx);

    std::vector<std::uint16_t> words;
    pass2(lines, ctx, words);

    std::vector<std::uint8_t> bytes;
    bytes.reserve(words.size() * 2);
    for (std::uint16_t w : words) {
        std::uint8_t lo = static_cast<std::uint8_t>(w & 0xFF);
        std::uint8_t hi = static_cast<std::uint8_t>((w >> 8) & 0xFF);
        bytes.push_back(lo);
        bytes.push_back(hi);
    }

    return bytes;
}
