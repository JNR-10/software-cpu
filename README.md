# 🧠 Software CPU (C/C++) Project

## 📜 Overview
This project implements a **complete software CPU simulator** in C/C++.  
It includes a fully defined **Instruction Set Architecture (ISA)**, **Assembler**, and **Emulator** capable of loading and executing assembly programs such as *Hello World* and *Fibonacci Sequence*.

Our goal is to demonstrate how a CPU operates internally — from **fetch–decode–execute cycles** to **memory-mapped I/O**, including a visual schematic and well-documented ISA.

---

## 🧩 Components

### 🖥️ 1. CPU Schematic
A simple diagram illustrating the architecture of our software CPU — showing connections between:
- Registers  
- ALU (Arithmetic Logic Unit)  
- Control Unit  
- Memory and Bus  
- I/O devices  

Located at:  
📁 `docs/cpu_schematic.png`

---

### ⚙️ 2. Instruction Set Architecture (ISA)
Defines:
- Instruction format and encoding
- Addressing modes
- Supported instructions (arithmetic, control flow, memory, I/O)
- Flag semantics (Zero, Carry, Sign, Overflow, etc.)
- Memory map and device addresses

Documentation:  
📁 `docs/ISA.md`  
📁 `docs/memory_map.md`  

---

### 💻 3. Emulator
The **core CPU simulation engine** that executes machine code instruction-by-instruction.

Modules:
- **Registers** (`src/emulator/registers.hpp`)  
  Holds CPU state (AX, BX, PC, SP, FLAGS, etc.)
- **ALU** (`src/emulator/alu.cpp`)  
  Handles arithmetic and logical operations.
- **Memory** (`src/emulator/memory.cpp`)  
  Simulates RAM and memory-mapped I/O.
- **Control Unit** (`src/emulator/control_unit.cpp`)  
  Manages instruction sequencing (fetch, decode, execute).
- **CPU Core** (`src/emulator/cpu.cpp`)  
  Integrates all components, runs the main instruction cycle.

---

### 🧾 4. Assembler
A C++ tool that converts `.asm` source files into binary machine code compatible with our emulator.

Features:
- Tokenization and parsing
- Label resolution
- Numeric literal handling
- Code output in binary or hex format

Located at:  
📁 `src/assembler/assembler.cpp`  
📁 `src/assembler/parser.cpp`

---

### 🧪 5. Example Programs
Example assembly programs demonstrating ISA usage:
- `hello_world.asm`
- `fibonacci.asm`
- `timer_example.asm`

Each shows how code executes through **Fetch → Compute → Store** cycles.

Located at:  
📁 `src/programs/`

---

### 📹 6. Demo Video
A walkthrough explaining:
- CPU design and instruction execution
- Step-by-step Fibonacci execution trace
- Memory dump and flag updates

Located at:  
📁 `video/demo_fibonacci.mp4`

---

## 🗂️ Directory Structure

software-cpu/
├── README.md
├── .gitignore
│
├── docs/
│   ├── cpu_schematic.png
│   ├── ISA.md
│   ├── memory_map.md
│   └── design_notes.md
│
├── src/
│   ├── main.cpp
│   │
│   ├── emulator/
│   │   ├── cpu.cpp
│   │   ├── cpu.hpp
│   │   ├── alu.cpp
│   │   ├── alu.hpp
│   │   ├── memory.cpp
│   │   ├── memory.hpp
│   │   ├── control_unit.cpp
│   │   ├── control_unit.hpp
│   │   └── registers.hpp
│   │
│   ├── assembler/
│   │   ├── assembler.cpp
│   │   ├── assembler.hpp
│   │   └── parser.cpp
│   │
│   ├── programs/
│   │   ├── hello_world.asm
│   │   ├── fibonacci.asm
│   │   └── timer_example.asm
│   │
│   └── utils/
│       ├── logger.cpp
│       └── logger.hpp
│
├── include/
│   └── common.hpp
│
├── tests/
│   ├── test_alu.cpp
│   ├── test_memory.cpp
│   ├── test_cpu.cpp
│   └── test_assembler.cpp
│
├── examples/
│   ├── demo_fibonacci_run.txt
│   ├── demo_hello_world.txt
│   └── sample_memory_dump.txt
│
├── build/
│
└── video/
    └── demo_fibonacci.mp4



---

## 🚀 Build & Run Instructions

### 🧰 Using `g++`
```bash
# From project root
cd software-cpu

# Compile the emulator and assembler
g++ src/main.cpp src/emulator/*.cpp src/assembler/*.cpp src/utils/*.cpp -Iinclude -o build/software_cpu

# Run with an assembly program
./build/software_cpu src/programs/fibonacci.asm
