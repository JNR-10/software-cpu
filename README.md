# Software CPU Simulator

A complete **16-bit CPU simulator** with assembler, emulator, and interactive web-based trace viewer. This project demonstrates how a CPU operates internally through fetch-decode-execute cycles, memory management, and I/O operations.

## Features

- **Complete ISA Implementation** - 25 instructions with 6 addressing modes
- **Full Assembler** - Converts assembly code to machine code
- **CPU Emulator** - Simulates 16-bit processor with registers, ALU, and memory
- **Interactive Trace Viewer** - Web-based visualization of CPU execution
- **Comprehensive Testing** - Unit tests and integration tests
- **Example Programs** - Hello World, Fibonacci, and more
- **Debug Tools** - Step-by-step execution and memory inspection

## Quick Start Demo

**One-command demo for new users:**
**Note: All commands are for Mac/Linux**
```bash
# Clone the repository
git clone <https://github.com/JNR-10/software-cpu>
cd software-cpu

#Download these dependices
WSL:
  sudo apt-get install -y jq
  sudo apt-get install -y dos2unix
Mac:
  brew install jq 
  brew install dos2unix

# Run the complete interactive demo
chmod +x demo.sh
dos2unix ./scripts/*.sh
dos2unix ./demo.sh
./demo.sh
```

This will:
1. Build the entire project
2. Run unit tests
3. Generate execution traces for multiple programs
4. Launch the interactive web-based trace viewer
5. Open your browser automatically to visualize CPU execution

## Architecture Overview

### CPU Components
- **16-bit Architecture** with 4 general-purpose registers (R0-R3)
- **ALU** supporting arithmetic, logical, and shift operations
- **Memory Management** with 64KB address space
- **Stack Operations** with dedicated stack pointer
- **Memory-Mapped I/O** for device communication
- **Flag Register** (Zero, Negative, Carry, Overflow)

### Instruction Set
- **25 Instructions**: NOP, HALT, MOV, LOAD, STORE, ADD, SUB, AND, OR, XOR, CMP, SHL, SHR, JMP, JZ, JNZ, JC, JNC, JN, CALL, RET, PUSH, POP, IN, OUT
- **6 Addressing Modes**: Register, Immediate, Direct, Register Indirect, Register+Offset, PC-Relative
- **Complete Control Flow**: Conditional jumps, subroutine calls, stack operations

## Project Structure

```
software-cpu/
├──  demo.sh                    # One-command demo script
├──  README.md                  # This file
├──  Makefile                   # Build system
│
├──  docs/                      # Documentation
│   ├── ISA.md                   # Instruction Set Architecture
│   ├── memory_map.md            # Memory layout
│   └── phase*.md                # Development phases
│
├── src/                       # Source code
│   ├── main.cpp                 # Main application
│   ├── assembler/               # Assembly → Machine code
│   ├── emulator/                # CPU simulation engine
│   └── programs/                # Example assembly programs
│
├── tests/                     # Test suites
│   └── assembly/                # Assembly test programs
│
├── trace_viewer/              # Web-based trace visualization
│   ├── index.html               # Trace viewer interface
│   ├── script.js                # Visualization logic
│   └── style.css                # Styling
│
├── scripts/                   # Utility scripts
└── build/                     # Build artifacts and traces
```

## Usage Examples

### Libraries/Dependcies needed
sudo apt-get install -y jq
sudo apt-get install -y dos2unix

### Basic Operations

```bash
# Build everything
make all

# Run unit tests
make test

# Assemble a program
dos2unix ./bin/software-cpu assemble src/programs/fibonacci.asm build/fib.bin
./bin/software-cpu assemble src/programs/fibonacci.asm build/fib.bin

# Run a program
dos2unix ./bin/software-cpu run build/fib.bin
./bin/software-cpu run build/fib.bin

# Interactive debugging
dos2unix ./bin/software-cpu debug build/fib.bin
./bin/software-cpu debug build/fib.bin
```

### Quick Program Testing

```bash
# Test integration (simple ADD operations)
dos2unix ./scripts/run_general.sh tests/assembly/test_integration.asm
./scripts/run_general.sh tests/assembly/test_integration.asm

# Run Fibonacci calculation
dos2unix ./scripts/run_general.sh src/programs/fibonacci.asm
./scripts/run_general.sh src/programs/fibonacci.asm

# Test comprehensive instruction set
dos2unix ./scripts/run_general.sh tests/assembly/test_phase4b.asm
./scripts/run_general.sh tests/assembly/test_phase4b.asm
```

### Trace Generation & Visualization

```bash
# Generate execution trace
dos2unix ./scripts/run_general_with_trace.sh src/programs/fibonacci.asm fib_trace.json
./scripts/run_general_with_trace.sh src/programs/fibonacci.asm fib_trace.json

#You can view the trace in 2 places:
#1. The command line
#2. build/traces/programname_time.json
```

## Example Programs

| Program | Description | Demonstrates |
|---------|-------------|-------------|
| `hello_world.asm` | Simple HALT instruction | Basic program structure |
| `fibonacci.asm` | Fibonacci sequence calculation | Loops, arithmetic, memory |
| `timer_example.asm` | Timer functionality | Memory-mapped I/O |

## Interactive Trace Viewer

The web-based trace viewer provides:
- **Step-by-step execution** visualization
- **Register value tracking** in real-time
- **Instruction decoding** display
- **Memory access patterns** monitoring
- **Interactive timeline** with slider control

### Using the Trace Viewer
1. Run the demo.sh file
2. Auto starts the webserver
3. Select which program to trace
4. Use the slider to step through execution cycles
5. Watch registers and instructions change in real-time

## Development

### Build Requirements
- **C++17** compatible compiler (g++, clang++)
- **Make** build system
- **Python 3** (for trace viewer web server)

### Build Commands
```bash
make all          # Build everything
make test         # Build and run tests
make clean        # Clean build artifacts
```

### Testing
```bash
# Unit tests
make test

# Integration tests
dos2unix ./scripts/run_general.sh tests/assembly/test_integration.asm
./scripts/run_general.sh tests/assembly/test_integration.asm

# Error handling tests
dos2unix ./scripts/run_general.sh tests/assembly/test_error.asm
./scripts/run_general.sh tests/assembly/test_error.asm
```
