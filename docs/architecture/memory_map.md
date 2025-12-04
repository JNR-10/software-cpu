
# Memory Map - Phase 2: Function Calls & Stack Frames

This document describes the 16-bit address space layout of the Software CPU, with enhanced details for function calls, stack frames, and recursion support.

The CPU uses 16-bit addresses, giving a total address space of:

- `0x0000–0xFFFF` (64 KiB)

Program code, data, stack, and memory-mapped I/O share this unified address space.

---

## 1. Global Layout

Complete memory layout:

```text
Address Range      Description
-----------------  -------------------------------------------
0x0000–0x0FFF      Data Segment (4 KiB) – global variables
0x1000–0x7FFF      Stack Segment (28 KiB) – grows downward
0x8000–0xEFFF      Code Segment (28 KiB) – program code
0xF000–0xF0FF      Memory-mapped I/O (256 bytes)
0xF100–0xFFFF      Reserved (e.g., reset vector, ROM)
```

The emulator will enforce any special behavior associated with these regions (e.g. I/O side effects).

---

## 2. Data Segment (0x0000–0x0FFF)

- Size: 4 KiB
- Usage:
  - Global variables
  - Static data structures
  - Program constants
- This region is accessed using direct addressing or absolute addresses

---

## 3. Stack Segment (0x1000–0x7FFF)

- Size: 28 KiB
- **Stack grows downward** (toward lower addresses)
- Initial Stack Pointer: `SP = 0x7FFF`
- Stack grows from `0x7FFF` down toward `0x1000`

### 3.1 Stack Operations

The stack is used for:
- Function call return addresses (`CALL` instruction)
- Saved register values (`PUSH`/`POP` instructions)
- Local variables
- Function parameters
- Stack frames for recursion

### 3.2 Stack Frame Structure

Each function call creates a stack frame with the following layout (growing downward):

```text
Higher Addresses (0x7FFF)
┌─────────────────────────┐
│   Previous Stack Frame  │
├─────────────────────────┤
│   Return Address (PC)   │  ← Pushed by CALL instruction
├─────────────────────────┤
│   Saved Frame Pointer   │  ← Optional: saved R3 (FP)
├─────────────────────────┤
│   Saved Registers       │  ← Callee-saved registers
├─────────────────────────┤
│   Local Variables       │  ← Function local data
├─────────────────────────┤
│   Function Arguments    │  ← Arguments for next call
└─────────────────────────┘  ← Current SP
Lower Addresses (0x1000)
```

### 3.3 Calling Convention (ABI)

**Register Usage:**
- `R0`: First argument / Return value
- `R1`: Second argument / Temporary
- `R2`: Third argument / Temporary
- `R3`: Frame Pointer (FP) - points to current frame base

**Caller-Saved Registers:**
- `R0`, `R1`, `R2`: Caller must save these before calling a function if their values are needed after the call

**Callee-Saved Registers:**
- `R3` (FP): Callee must save and restore this register

**Stack Management:**
- `CALL` instruction: Pushes return address (PC) onto stack
- `RET` instruction: Pops return address from stack into PC
- Function prologue: Save FP, set new FP, allocate local space
- Function epilogue: Restore FP, deallocate local space

**Example Function Call Sequence:**

```assembly
; Caller code
MOV R0, #5          ; Pass argument in R0
CALL factorial      ; Push PC, jump to factorial
; Return value is in R0

; Callee (factorial) prologue
factorial:
    PUSH R3         ; Save caller's frame pointer
    MOV R3, SP      ; Set new frame pointer
    ; ... function body ...
    
; Callee epilogue
    MOV SP, R3      ; Restore stack pointer
    POP R3          ; Restore caller's frame pointer
    RET             ; Pop return address, jump back
```

---

## 4. Code Segment (0x8000–0xEFFF)

- Size: 28 KiB
- Used for program code and read-only data
- The emulator loads assembled programs into this region
- Entry point convention: `PC = 0x8000` at reset/startup
- Code is position-independent when using PC-relative addressing

---

## 5. Memory-Mapped I/O (0xF000–0xF0FF)

This region is reserved for I/O devices accessed as if they were memory.

Example assignments:

- `0xF000`: Output data register
  - Writing a byte/word here causes the emulator to display or log the value (e.g. character output).

- `0xF001`: Input data register
  - Reading from this address returns input data (e.g. keyboard or stdin in the emulator).

- `0xF010–0xF01F`: Timer registers
  - Reserved for future timer functionality used in example programs.

Any access in this range should be interpreted by the emulator as an I/O operation, not normal RAM.

---

## 6. Reserved / Vectors (0xF100–0xFFFF)

The top of memory is reserved for future extensions such as:

- Interrupt and exception vectors
- Reset vector (initial `PC` value)
- ROM or fixed configuration data

---

## 7. Memory Layout for Recursion Example

For a recursive factorial function `factorial(5)`, the stack evolves as follows:

**Initial State:**
```
SP = 0x7FFF (empty stack)
```

**After `factorial(5)` call:**
```
0x7FFD: Return address (caller's PC)
0x7FFB: Saved FP
0x7FF9: n=5
SP = 0x7FF9
```

**After `factorial(4)` call (recursive):**
```
0x7FFD: Return address (main)
0x7FFB: Saved FP (main)
0x7FF9: n=5
0x7FF7: Return address (factorial)
0x7FF5: Saved FP (factorial)
0x7FF3: n=4
SP = 0x7FF3
```

This pattern continues until the base case `factorial(1)` is reached, then unwinds as each function returns.




