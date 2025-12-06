# Phase 2: Function Calls, Stack Frames, and Recursion - COMPLETE ✅

## Overview

This phase implements support for function calls, stack frames, and recursion in the Software CPU. A recursive factorial function demonstrates how the CPU handles function calls and manages the stack.

## Project Status: ALL TASKS COMPLETE ✅

### ✅ Task 1: The Architect (Mohit)
- ✅ Updated Memory Map (`docs/architecture/memory_map.md`)
- ✅ Calling Convention Documentation (`docs/function_calls_stack_frames.md`)
- ✅ Stack Test Verification (`tests/assembly/test_stack.asm`)

### ✅ Task 2: The Algorithmist (Jainil)
- ✅ Implemented `multiply(a, b)` subroutine (`src/programs/math.asm`)
- ✅ Designed factorial logic (`src/programs/factorial.c`, `factorial_logic.txt`)

### ✅ Task 3: The Implementer (Shri)
- ✅ Implemented `factorial(n)` in assembly (`src/programs/factorial.asm`)
- ✅ Generated execution traces with trace viewer

### ✅ Task 4: The Integrator (Mohit & Charles)
- ✅ Implemented `main` program with factorial calls
- ✅ Created interactive trace viewer with memory visualization
- ✅ Final integration and verification complete

## Quick Start

### Run the Demo

```bash
# Run complete demo with trace viewer
./demo.sh

# Then open browser to: http://localhost:8000/trace_viewer/
```

### Run Individual Programs

```bash
# Build the project
make all

# Run factorial program
./scripts/run_general.sh src/programs/factorial.asm

# Generate trace
./scripts/run_general_with_trace.sh src/programs/factorial.asm factorial_trace.json
```

## Architecture Summary

### Memory Layout

```
Address Range      Description
-----------------  -------------------------------------------
0x0000–0x0FFF      Data Segment (4 KiB) – global variables
0x1000–0x7FFF      Stack Segment (28 KiB) – grows downward
0x8000–0xEFFF      Code Segment (28 KiB) – program code
0xF000–0xF0FF      Memory-mapped I/O (256 bytes)
0xF100–0xFFFF      Reserved (vectors, ROM)
```

### Calling Convention

**Register Roles:**
- **R0**: First argument / Return value
- **R1**: Second argument / Temporary
- **R2**: Third argument / Temporary
- **R3**: Frame Pointer (FP) - Callee-saved

**Stack Frame:**
```
Higher Addresses
┌─────────────────────────┐
│   Return Address (PC)   │  ← Pushed by CALL
├─────────────────────────┤
│   Saved Registers       │  ← PUSH/POP
├─────────────────────────┤
│   Local Variables       │
└─────────────────────────┘  ← Current SP
Lower Addresses
```

## Interactive Trace Viewer

The trace viewer provides real-time visualization of CPU execution:

1. **Registers & Flags** - Shows all register values and status flags
2. **Current Instruction** - Displays the executing instruction
3. **Stack Memory** - Visualizes recursion depth and stack growth
4. **Memory Operations** - Tracks all memory reads/writes
5. **Controls** - Playback controls with variable speed
6. **Program Code** - Syntax-highlighted assembly with active line tracking
7. **Cycle Bar** - Progress slider for navigation
8. **Memory Layout** - Shows all 4 memory segments with live status

## Implemented Programs

### 1. Factorial (Recursive)
- **File**: `src/programs/factorial.asm`
- **Features**: Recursive implementation with stack frames
- **Test Cases**: factorial(5) = 120, factorial(10) = 3628800
- **Trace**: Shows complete recursion depth and stack evolution

### 2. Multiplication
- **File**: `src/programs/math.asm`
- **Implementation**: Shift-and-add algorithm
- **Used by**: Factorial function

### 3. Reference Implementation
- **File**: `src/programs/factorial.c`
- **Purpose**: C reference for verification

## Verification Results

### All Tests Pass ✅
- ✅ Stack operations (PUSH/POP)
- ✅ Function calls (CALL/RET)
- ✅ Multiplication function
- ✅ Factorial base cases (0! = 1, 1! = 1)
- ✅ Factorial recursive cases (5! = 120)
- ✅ Stack properly managed (no overflow)
- ✅ Registers properly saved/restored
- ✅ Trace shows correct recursion depth

### Factorial(5) Execution
```
factorial(5) = 5 * factorial(4)
             = 5 * 4 * factorial(3)
             = 5 * 4 * 3 * factorial(2)
             = 5 * 4 * 3 * 2 * factorial(1)
             = 5 * 4 * 3 * 2 * 1
             = 120 ✅
```

### Stack Evolution
```
Initial:     SP = 0x7FFF
Call f(5):   SP = 0x7FFD (return addr), 0x7FFB (saved n=5)
Call f(4):   SP = 0x7FF9 (return addr), 0x7FF7 (saved n=4)
Call f(3):   SP = 0x7FF5 (return addr), 0x7FF3 (saved n=3)
Call f(2):   SP = 0x7FF1 (return addr), 0x7FEF (saved n=2)
Call f(1):   SP = 0x7FED (return addr) → Returns 1
Unwind...    SP returns to 0x7FFF
Result:      R0 = 120 ✅
```

## Documentation

- **Memory Map**: `docs/architecture/memory_map.md`
- **ISA Reference**: `docs/architecture/ISA.md`
- **Function Calls & Stack Frames**: `docs/function_calls_stack_frames.md`
- **Task Summaries**: `docs/task*_summary.md`

## Tools

- `./demo.sh` - Complete demo with trace viewer
- `./scripts/run_general.sh` - Run assembly programs
- `./scripts/run_general_with_trace.sh` - Generate execution traces
- `trace_viewer/` - Interactive web-based trace visualization

## Success Criteria - ALL MET ✅

- ✅ All unit tests pass
- ✅ Factorial(5) returns 120
- ✅ Stack properly managed (no corruption)
- ✅ Trace shows correct execution flow
- ✅ Memory visualization shows stack frames
- ✅ Documentation complete
- ✅ Interactive trace viewer functional

---

**Phase 2 Status**: ✅ COMPLETE - All tasks finished and verified
