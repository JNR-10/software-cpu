# Task 3 - The Implementer: Implementation Summary

**Assigned to:** Shri  
**Focus:** Recursive Factorial Implementation and Execution Tracing

## Completed Deliverables

### 1. Task 3.1: Implement Recursive Factorial in Assembly ✅

**File:** `src/programs/factorial.asm`

**Implementation Details:**
The factorial function was implemented using the standard recursive definition: `n! = n * (n-1)!`. Since the CPU lacks a hardware `MUL` instruction, we integrated the software multiplication routine (shift-and-add algorithm) from Task 2.

**Logic Flow:**
1.  **Base Case:**
    - Check if `n <= 1`.
    - If yes, return 1 in `R0`.
    - Use `CMP R0, #1` followed by `JZ` and `JC`.

2.  **Recursive Case:**
    - Push `n` (R0) onto the stack to preserve it.
    - Decrement `n` (`SUB R0, #1`).
    - Call `factorial` recursively (`CALL factorial`).
    - **Stack Frame:** Upon return, the result of `(n-1)!` is in `R0`.
    - Pop original `n` back into `R1`.

3.  **Multiplication:**
    - Call `multiply` with `R0` (result of recursive call) and `R1` (original n).
    - `R0 = R0 * R1`.
    - Return `R0`.

**Code Snippet:**
```assembly
factorial:
    ; Base case: if n <= 1, return 1
    CMP R0, #1
    JZ base_case
    JC base_case
    
    ; Recursive case
    PUSH R0             ; Save n
    SUB R0, #1          ; n - 1
    CALL factorial      ; factorial(n-1)
    POP R1              ; Restore n to R1
    
    ; Calculate n * factorial(n-1)
    CALL multiply       ; R0 = R0 * R1
    RET
    
base_case:
    MOV R0, #1          ; return 1
    RET
```

### 2. Task 3.2: Generate Execution Traces ✅

**Trace File:** `factorial_trace.json`

We generated a full execution trace to verify the recursion depth and stack management.

**Command Used:**
```bash
./scripts/run_general_with_trace.sh src/programs/factorial.asm factorial_trace.json
```

## Verification Results

### 1. Functional Correctness
We calculated `factorial(5)`:
- **Expected Result:** 5! = 120 = `0x0078`
- **Actual Result:** `R0` contained `0x0078` at the end of execution.
- **Memory Check:** The result was successfully stored at memory address `0x0200`.

### 2. Stack Management
- **Initial SP:** `0x7FFF`
- **During Recursion:** SP decreased by 2 for each PUSH and CALL (4 bytes per level).
- **Final SP:** `0x7FFF`
- **Conclusion:** The stack was perfectly balanced. Every PUSH had a matching POP, and every CALL had a matching RET.

### 3. Integration
- The `factorial` function successfully called the `multiply` subroutine.
- Register preservation rules were followed (R0-R3 usage).

## Files Created/Modified

1.  ✅ `src/programs/factorial.asm` - The complete source code.
2.  ✅ `factorial_trace.json` - Execution history for debugging.
3.  ✅ `PHASE2_README.md` - Updated status to COMPLETE.

## Next Steps for Team

### For Charles (Integrator - Task 4)
- The `factorial` function is verified and ready for the main integration.
- You can use `src/programs/factorial.asm` as the core logic.
- Focus on the "Memory Snapshot Visualization" to show the stack frames visually.
- Verify the final integration passes all checks.

## Conclusion

Task 3 (The Implementer) is **COMPLETE** and **VERIFIED**. The recursive factorial logic works correctly on the software CPU, effectively utilizing the stack for recursion and software routines for arithmetic.
