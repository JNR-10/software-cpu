# Function Calls & Stack Frames

## Overview

This document describes how function calls, stack frames, and recursion work in the Software CPU architecture. This is essential for understanding Phase 2 of the project, which implements a recursive factorial function.

---

## 1. Calling Convention (ABI)

The Application Binary Interface (ABI) defines how functions interact with each other through registers and the stack.

### 1.1 Register Roles

| Register | Role | Saved By | Description |
|----------|------|----------|-------------|
| `R0` | Argument 1 / Return Value | Caller | First function argument, function return value |
| `R1` | Argument 2 / Temporary | Caller | Second argument, scratch register |
| `R2` | Argument 3 / Temporary | Caller | Third argument, scratch register |
| `R3` | Frame Pointer (FP) | Callee | Points to current stack frame base |
| `SP` | Stack Pointer | Hardware | Points to top of stack (managed by PUSH/POP/CALL/RET) |
| `PC` | Program Counter | Hardware | Points to next instruction (saved by CALL) |

### 1.2 Caller-Saved vs Callee-Saved

**Caller-Saved Registers (R0, R1, R2):**
- The **caller** must save these registers before calling a function if their values are needed after the call
- The **callee** is free to modify these registers without saving them
- Used for passing arguments and temporary values

**Callee-Saved Registers (R3/FP):**
- The **callee** must save and restore these registers if it uses them
- The **caller** can assume these registers are preserved across function calls
- R3 is used as the Frame Pointer to maintain stack frame structure

---

## 2. Stack Frame Layout

Each function call creates a **stack frame** on the stack. The stack grows **downward** (from high addresses to low addresses).

### 2.1 Stack Frame Structure

```text
Higher Addresses
┌─────────────────────────────┐
│   Caller's Stack Frame      │
├─────────────────────────────┤  ← FP of caller
│   Return Address            │  ← Pushed by CALL instruction
├─────────────────────────────┤
│   Saved Frame Pointer (R3)  │  ← Pushed by callee prologue
├─────────────────────────────┤  ← New FP points here
│   Saved Registers (if any)  │  ← Callee-saved registers
├─────────────────────────────┤
│   Local Variables           │  ← Space for local data
├─────────────────────────────┤
│   Arguments for next call   │  ← Arguments passed to callees
└─────────────────────────────┘  ← Current SP
Lower Addresses
```

### 2.2 Frame Pointer (FP) Usage

The Frame Pointer (R3) provides a stable reference point within the current stack frame:
- Points to a fixed location in the current frame
- Allows access to local variables and parameters using fixed offsets
- Remains constant during function execution (unlike SP which changes with PUSH/POP)

**Accessing Stack Frame Elements:**
```assembly
; Assuming FP points to saved FP location
LOAD R0, [R3, #2]    ; Load return address
LOAD R1, [R3, #-2]   ; Load first local variable
LOAD R2, [R3, #-4]   ; Load second local variable
```

---

## 3. Function Call Sequence

### 3.1 Calling a Function

**Caller's Responsibilities:**

1. **Save caller-saved registers** (if needed):
   ```assembly
   PUSH R1    ; Save R1 if we need it after the call
   PUSH R2    ; Save R2 if we need it after the call
   ```

2. **Pass arguments**:
   ```assembly
   MOV R0, #5    ; First argument in R0
   ```

3. **Call the function**:
   ```assembly
   CALL function_name    ; Pushes PC, jumps to function
   ```

4. **Retrieve return value**:
   ```assembly
   ; Return value is in R0
   MOV R1, R0    ; Save return value if needed
   ```

5. **Restore caller-saved registers**:
   ```assembly
   POP R2     ; Restore R2
   POP R1     ; Restore R1
   ```

### 3.2 Function Prologue (Entry)

**Callee's Responsibilities on Entry:**

1. **Save the caller's frame pointer**:
   ```assembly
   PUSH R3    ; Save caller's FP
   ```

2. **Set up new frame pointer**:
   ```assembly
   MOV R3, SP    ; FP points to current stack top
   ```

3. **Allocate space for local variables** (if needed):
   ```assembly
   SUB SP, #4    ; Allocate 2 words (4 bytes) for locals
   ```

4. **Save callee-saved registers** (if used):
   ```assembly
   ; (R3 already saved above)
   ```

### 3.3 Function Epilogue (Exit)

**Callee's Responsibilities Before Return:**

1. **Place return value in R0**:
   ```assembly
   MOV R0, result    ; Return value in R0
   ```

2. **Deallocate local variables**:
   ```assembly
   MOV SP, R3    ; Restore SP to FP (deallocates locals)
   ```

3. **Restore caller's frame pointer**:
   ```assembly
   POP R3    ; Restore caller's FP
   ```

4. **Return to caller**:
   ```assembly
   RET    ; Pops return address into PC
   ```

---

## 4. Complete Function Example

Here's a complete example of a simple function that adds two numbers:

```assembly
; Function: add(a, b) -> a + b
; Arguments: R0 = a, R1 = b
; Returns: R0 = a + b

add:
    ; Prologue
    PUSH R3         ; Save caller's FP
    MOV R3, SP      ; Set new FP
    ; No local variables needed
    
    ; Function body
    ADD R0, R1      ; R0 = R0 + R1
    
    ; Epilogue
    MOV SP, R3      ; Restore SP
    POP R3          ; Restore caller's FP
    RET             ; Return to caller

; Caller code
main:
    MOV R0, #10     ; First argument
    MOV R1, #20     ; Second argument
    CALL add        ; Call function
    ; R0 now contains 30
```

---

## 5. Recursion

Recursion occurs when a function calls itself. Each recursive call creates a new stack frame.

### 5.1 Recursive Function Structure

```assembly
recursive_function:
    ; Prologue
    PUSH R3
    MOV R3, SP
    
    ; Check base case
    CMP R0, #base_value
    JZ base_case
    
    ; Recursive case
    ; 1. Save current argument
    PUSH R0
    
    ; 2. Prepare argument for recursive call
    SUB R0, #1
    
    ; 3. Make recursive call
    CALL recursive_function
    
    ; 4. Restore argument
    POP R1          ; Original argument in R1
    
    ; 5. Combine results
    ; ... process R0 (recursive result) with R1 (original arg) ...
    
    JMP epilogue
    
base_case:
    ; Base case: return simple value
    MOV R0, #base_return_value
    
epilogue:
    ; Epilogue
    MOV SP, R3
    POP R3
    RET
```

### 5.2 Stack Growth During Recursion

For `factorial(5)`, the stack grows as follows:

```text
Call factorial(5):
  SP: 0x7FFD - Return address
  SP: 0x7FFB - Saved FP
  SP: 0x7FF9 - n=5

Call factorial(4):
  SP: 0x7FF7 - Return address
  SP: 0x7FF5 - Saved FP  
  SP: 0x7FF3 - n=4

Call factorial(3):
  SP: 0x7FF1 - Return address
  SP: 0x7FEF - Saved FP
  SP: 0x7FED - n=3

... and so on until base case
```

Each return pops one frame, unwinding the stack back to the original caller.

---

## 6. Stack Pointer Management

### 6.1 PUSH Operation

```
Before PUSH R0:
  SP = 0x7FFF
  
PUSH R0 executes:
  1. SP = SP - 2     (pre-decrement)
  2. MEM[SP] = R0    (store value)
  
After PUSH R0:
  SP = 0x7FFD
  MEM[0x7FFD] = R0
```

### 6.2 POP Operation

```
Before POP R0:
  SP = 0x7FFD
  MEM[0x7FFD] = value
  
POP R0 executes:
  1. R0 = MEM[SP]    (load value)
  2. SP = SP + 2     (post-increment)
  
After POP R0:
  R0 = value
  SP = 0x7FFF
```

### 6.3 CALL Operation

```
Before CALL target:
  PC = 0x8010
  SP = 0x7FFF
  
CALL target executes:
  1. SP = SP - 2         (pre-decrement)
  2. MEM[SP] = PC        (save return address)
  3. PC = target         (jump to function)
  
After CALL:
  PC = target
  SP = 0x7FFD
  MEM[0x7FFD] = 0x8010
```

### 6.4 RET Operation

```
Before RET:
  SP = 0x7FFD
  MEM[0x7FFD] = 0x8010
  
RET executes:
  1. PC = MEM[SP]    (restore return address)
  2. SP = SP + 2     (post-increment)
  
After RET:
  PC = 0x8010
  SP = 0x7FFF
```

---

## 7. Best Practices

### 7.1 Always Balance Stack Operations
- Every PUSH must have a corresponding POP
- Every CALL must have a corresponding RET
- Unbalanced stack operations lead to crashes or incorrect behavior

### 7.2 Use Frame Pointer for Complex Functions
- Functions with local variables should use FP
- FP provides stable reference for accessing locals
- Simplifies stack management

### 7.3 Document Register Usage
- Clearly document which registers are used by each function
- Specify which registers are preserved
- Document argument passing convention

### 7.4 Initialize Stack Pointer
- Always initialize SP before first function call
- Typical initialization: `MOV SP, #0x7FFF`

### 7.5 Check Stack Overflow
- Be aware of stack limits (0x1000 - 0x7FFF)
- Deep recursion can overflow the stack
- Consider iterative alternatives for very deep recursion

---

## 8. Common Pitfalls

### 8.1 Forgetting to Save Registers
```assembly
; WRONG: R1 is lost
function:
    MOV R1, #100    ; Overwrites caller's R1
    RET

; CORRECT: Save and restore R1
function:
    PUSH R1         ; Save R1
    MOV R1, #100    ; Use R1
    POP R1          ; Restore R1
    RET
```

### 8.2 Incorrect Stack Cleanup
```assembly
; WRONG: Stack not balanced
function:
    PUSH R3
    PUSH R1
    ; ... code ...
    POP R3          ; Wrong order!
    RET

; CORRECT: LIFO order
function:
    PUSH R3
    PUSH R1
    ; ... code ...
    POP R1          ; Restore in reverse order
    POP R3
    RET
```

### 8.3 Missing Frame Pointer Save
```assembly
; WRONG: FP not saved
function:
    MOV R3, SP      ; Overwrites caller's FP
    ; ... code ...
    RET

; CORRECT: Save and restore FP
function:
    PUSH R3         ; Save caller's FP
    MOV R3, SP      ; Set new FP
    ; ... code ...
    POP R3          ; Restore caller's FP
    RET
```

---

## 9. Summary

**Key Points:**
1. **R0** is used for the first argument and return value
2. **R3** is the Frame Pointer, must be saved by callee
3. **CALL** pushes return address, **RET** pops it
4. Stack grows **downward** from 0x7FFF
5. Each function call creates a new stack frame
6. Recursion creates multiple stack frames
7. Always balance PUSH/POP and CALL/RET operations

**Function Template:**
```assembly
function_name:
    ; Prologue
    PUSH R3              ; Save FP
    MOV R3, SP           ; Set new FP
    SUB SP, #locals      ; Allocate locals (if needed)
    
    ; Function body
    ; ... your code here ...
    
    ; Epilogue
    MOV SP, R3           ; Deallocate locals
    POP R3               ; Restore FP
    RET                  ; Return to caller
```
