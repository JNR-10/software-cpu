#pragma once

#include <cstdint>
#include <string>
#include <vector>

// Assemble a small subset of the Phase 1 ISA.
// Returns little-endian bytes of the resulting machine code.
std::vector<std::uint8_t> assemble(const std::string& source);
