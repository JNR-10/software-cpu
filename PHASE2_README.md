# Phase 2: Function Calls, Stack Frames, and Recursion

## Overview

This phase implements support for function calls, stack frames, and recursion in the Software CPU. The goal is to implement a recursive factorial function and demonstrate how the CPU handles function calls and manages the stack.

## Project Status

### ✅ Task 1: The Architect (Mohit) - COMPLETE

**Deliverables:**
1. ✅ Updated Memory Map (`docs/architecture/memory_map.md`)
2. ✅ Calling Convention Documentation (`docs/function_calls_stack_frames.md`)
3. ✅ Stack Test Verification (`tests/assembly/test_stack.asm`)

**Summary:** All architecture documentation is complete and tested. The memory layout, calling convention, and stack mechanics are fully defined and verified.

### 🔄 Task 2: The Algorithmist (Jainil) - PENDING

**Deliverables:**
1. ⏳ Implement `multiply(a, b)` subroutine
2. ⏳ Design factorial logic (C reference and assembly pseudocode)

### 🔄 Task 3: The Implementer (Shri) - PENDING

**Deliverables:**
1. ⏳ Implement `factorial(n)` in assembly
2. ⏳ Generate execution traces

### 🔄 Task 4: The Integrator (Charles) - PENDING

**Deliverables:**
1. ⏳ Implement `main` program
2. ⏳ Create memory snapshot visualization
3. ⏳ Final integration and verification

## Quick Start

### Running the Stack Test

```bash
# Build the project
make all

# Run the stack test
./scripts/run_general.sh tests/assembly/test_stack.asm

# Generate trace
./scripts/run_general_with_trace.sh tests/assembly/test_stack.asm stack_trace.json
```

**Expected Output:**
- R0 = 0x0000 (success code)
- SP = 0x7FFF (stack restored)
- Memory[0x0100] = 0x0000 (success indicator)

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

### Function Template

```assembly
function_name:
    ; Prologue (if needed)
    PUSH R3              ; Save registers
    
    ; Function body
    ; ... your code ...
    ; Return value in R0
    
    ; Epilogue
    POP R3               ; Restore registers
    RET                  ; Return to caller
```

## Documentation

### Architecture Documents
- **Memory Map**: `docs/architecture/memory_map.md`
- **ISA**: `docs/architecture/ISA.md`
- **Function Calls & Stack Frames**: `docs/function_calls_stack_frames.md`

### Task Summaries
- **Task 1 Summary**: `docs/task1_architect_summary.md`

### Test Programs
- **Stack Test**: `tests/assembly/test_stack.asm` - Verifies PUSH/POP/CALL/RET

## Implementation Guidelines

### For Multiplication (Task 2.1)

Since the CPU lacks a MUL instruction, implement using:

**Option 1: Repeated Addition**
```assembly
multiply:
    ; R0 = a, R1 = b
    ; Returns R0 = a * b
    MOV R2, #0          ; result = 0
loop:
    CMP R1, #0
    JZ done
    ADD R2, R0          ; result += a
    SUB R1, #1          ; b--
    JMP loop
done:
    MOV R0, R2          ; return result
    RET
```

**Option 2: Shift-and-Add** (more efficient)
```assembly
multiply:
    ; R0 = a, R1 = b
    ; Returns R0 = a * b
    MOV R2, #0          ; result = 0
loop:
    CMP R1, #0
    JZ done
    ; Check if b is odd
    MOV R3, R1
    AND R3, #1
    JZ skip_add
    ADD R2, R0          ; result += a
skip_add:
    SHL R0, #1          ; a <<= 1
    SHR R1, #1          ; b >>= 1
    JMP loop
done:
    MOV R0, R2          ; return result
    RET
```

### For Factorial (Task 3.1)

**Recursive Structure:**
```assembly
factorial:
    ; R0 = n
    ; Returns R0 = n!
    
    ; Base case: if n <= 1, return 1
    CMP R0, #1
    JZ base_case
    JC base_case        ; if n < 1
    
    ; Recursive case
    PUSH R0             ; Save n
    SUB R0, #1          ; n - 1
    CALL factorial      ; factorial(n-1)
    POP R1              ; Restore n to R1
    
    ; R0 = factorial(n-1)
    ; R1 = n
    ; Need to compute R0 = n * factorial(n-1)
    CALL multiply       ; R0 = R1 * R0
    RET
    
base_case:
    MOV R0, #1          ; return 1
    RET
```

### For Main Program (Task 4.1)

```assembly
.org 0x8000

main:
    ; SP is automatically initialized to 0x7FFF
    
    ; Call factorial(5)
    MOV R0, #5
    CALL factorial
    
    ; Store result in memory
    MOV R1, #0x0200     ; Result address
    STORE R0, [R1]      ; Store factorial(5)
    
    HALT

; Include multiply and factorial functions here
```

## Testing Strategy

### Unit Tests
1. ✅ Stack operations (PUSH/POP)
2. ✅ Function calls (CALL/RET)
3. ⏳ Multiplication function
4. ⏳ Factorial function (base cases)
5. ⏳ Factorial function (recursive cases)

### Integration Tests
1. ⏳ Complete factorial program
2. ⏳ Verify result for factorial(5) = 120
3. ⏳ Trace analysis showing recursion depth
4. ⏳ Memory snapshot at deepest recursion

### Verification Checklist
- [ ] Multiplication works for various inputs
- [ ] Factorial(0) = 1
- [ ] Factorial(1) = 1
- [ ] Factorial(5) = 120
- [ ] Stack properly managed (no overflow)
- [ ] Registers properly saved/restored
- [ ] Trace shows correct recursion depth

## Known Limitations

### Assembler Constraints
1. **SP Register**: Cannot use SP in MOV instructions
   - Workaround: Use simpler calling conventions
   
2. **STORE Instruction**: Requires register indirect addressing
   - Must use: `STORE R0, [R1]` where R1 holds address
   - Cannot use: `STORE R0, 0x0100` directly

### Stack Constraints
- Stack size: 28 KiB (0x1000 to 0x7FFF)
- Each recursion level uses ~4-8 bytes
- Maximum practical recursion depth: ~3,500-7,000 levels
- Factorial(5) uses only 5 levels - well within limits

## Expected Results

### Factorial(5) Calculation
```
factorial(5) = 5 * factorial(4)
             = 5 * 4 * factorial(3)
             = 5 * 4 * 3 * factorial(2)
             = 5 * 4 * 3 * 2 * factorial(1)
             = 5 * 4 * 3 * 2 * 1
             = 120
```

### Stack Evolution
```
Initial:     SP = 0x7FFF

Call f(5):   SP = 0x7FFD (return addr)
             SP = 0x7FFB (saved n=5)

Call f(4):   SP = 0x7FF9 (return addr)
             SP = 0x7FF7 (saved n=4)

Call f(3):   SP = 0x7FF5 (return addr)
             SP = 0x7FF3 (saved n=3)

Call f(2):   SP = 0x7FF1 (return addr)
             SP = 0x7FEF (saved n=2)

Call f(1):   SP = 0x7FED (return addr)
             Returns 1

Unwind...    SP returns to 0x7FFF
Result:      R0 = 120
```

## Team Collaboration

### Communication
- Document any issues or discoveries
- Share test results
- Coordinate on interface between components

### Code Integration
- Use consistent naming conventions
- Follow the calling convention strictly
- Test each component independently first
- Integrate incrementally

### Version Control
- Commit working code frequently
- Use descriptive commit messages
- Tag major milestones

## Resources

### Documentation
- [Memory Map](docs/architecture/memory_map.md)
- [ISA Reference](docs/architecture/ISA.md)
- [Function Calls Guide](docs/function_calls_stack_frames.md)
- [Task 1 Summary](docs/task1_architect_summary.md)

### Example Programs
- `tests/assembly/test_stack.asm` - Stack operations
- `src/programs/fibonacci.asm` - Iterative example
- `src/programs/hello_world.asm` - Basic I/O

### Tools
- `./scripts/run_general.sh` - Run assembly program
- `./scripts/run_general_with_trace.sh` - Generate execution trace
- `./demo.sh` - Full demo with trace viewer

## Next Steps

1. **Jainil**: Implement multiplication function
2. **Shri**: Implement factorial function using multiplication
3. **Charles**: Create main program and integrate all components
4. **All**: Test, debug, and document results

## Success Criteria

- ✅ All unit tests pass
- ⏳ Factorial(5) returns 120
- ⏳ Stack properly managed (no corruption)
- ⏳ Trace shows correct execution flow
- ⏳ Memory snapshot shows stack frames
- ⏳ Documentation complete

---

**Phase 2 Status**: Task 1 Complete, Tasks 2-4 In Progress
