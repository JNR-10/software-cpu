# Phase 4B Implementation Changes

## 1. Test Suite Updates
- Created `tests/test_alu.cpp`: Comprehensive tests for ALU operations and flags.
- Created `tests/test_memory.cpp`: Tests for memory operations, I/O, and timer.
- Created `tests/test_cpu.cpp`: Tests for all instruction types and addressing modes.
- Created `tests/test_assembler.cpp`: Tests for assembler directives, literals, and error handling.
- Updated `Makefile` to include `test-all` target for running all unit tests.

## 2. Assembler Enhancements
- Added support for **Indirect Register Addressing** `[Reg]` (Mode 3).
- Added support for **Direct Addressing** `[#Addr]` or `[Label]` (Mode 2).
- Updated tokenizer to handle `[` and `]` characters.
- Updated parser and encoder to support these new addressing modes.

## 3. Example Programs
- **Fibonacci (`fibonacci.asm`)**: Updated to use `CMP`, `JZ`, `MOV`, and `SUB` for a proper loop structure.
- **Hello World (`hello_world.asm`)**: Updated to use `LOAD [Reg]`, `CMP`, `JZ`, and `STORE [Reg]` for string output.
- **Timer Example (`timer_example.asm`)**: Updated to use `MOV`, `STORE [Reg]`, `LOAD [Reg]`, and `JC` to demonstrate timer functionality.

## 4. Bug Fixes
- **ALU CMP/SUB**: Fixed Carry flag logic for subtraction. Now sets Carry=1 when a borrow occurs (a < b), matching standard unsigned comparison behavior.
- **CPU Timer**: Added `memory_.tick()` call in `CPU::step()` to ensure timer updates during execution.
- **Debug Output**: Updated `opcode_to_string` to include all opcodes, resolving "UNKNOWN" instruction names in debug traces.

## 5. Verification
- All unit tests passing (`make test-all`).
- All example programs assemble and run correctly.
- New regression test `tests/assembly/test_branch_flags.asm` verifies flag logic and branching.
