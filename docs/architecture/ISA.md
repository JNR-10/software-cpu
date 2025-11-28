
# Instruction Set Architecture (ISA)

This document describes the Instruction Set Architecture of the 16-bit Software CPU.

It summarizes:

- Registers and flags
- Instruction word format
- Addressing modes
- Core opcode set

---

## 1. Registers

### 1.1 Programmer-Visible Registers

- **General-Purpose Registers (GPRs)**
  - `R0`, `R1`, `R2`, `R3` (4 × 16-bit)
  - Used for arithmetic, logic, addressing, and temporaries.

- **Program Counter (`PC`, 16-bit)**
  - Holds the address of the *next instruction word* to fetch.

- **Stack Pointer (`SP`, 16-bit)**
  - Points to the *top of the stack* in memory.
  - Stack grows downward (pre-decrement on push, post-increment on pop).

- **Flags Register (`FLAGS`, 8-bit)**
  - Only the lower 4 bits are used:
    - Bit 0: `Z` – Zero
    - Bit 1: `N` – Negative
    - Bit 2: `C` – Carry
    - Bit 3: `V` – Overflow
    - Bits 4–7: reserved (0)

Total programmer-visible registers:

- `R0–R3`, `PC`, `SP`, `FLAGS`.

### 1.2 Internal (Microarchitectural) Registers

These are used internally by the CPU implementation:

- `IR` – Instruction Register (16-bit)
- `MAR` – Memory Address Register (16-bit)
- `MDR` – Memory Data Register (16-bit)
- `TMP` – Temporary register (optional)

---

## 2. Flags and Condition Codes

Most ALU operations update the flags: `ADD`, `SUB`, `AND`, `OR`, `XOR`, `CMP`, `SHL`, `SHR`.

- **Zero (`Z`)**
  - Set if result == `0x0000`.
  - Cleared otherwise.

- **Negative (`N`)**
  - Interprets result as signed 16-bit.
  - Set if bit 15 of the result is 1.
  - Cleared otherwise.

- **Carry (`C`)**
  - `ADD`: 1 if unsigned carry out (result > `0xFFFF`).
  - `SUB`: 1 if no borrow required (`a >= b` unsigned).
  - Shifts:
    - `SHL`: bit shifted out of MSB.
    - `SHR`: bit shifted out of LSB.

- **Overflow (`V`)**
  - Indicates signed overflow.
  - `ADD`: set if same-sign operands produce result with different sign.
  - `SUB`: set if signed subtraction overflows.
  - Cleared for logical ops where overflow is not meaningful.

Conditional branches (`JZ`, `JNZ`, `JC`, `JNC`, `JN`) read these flags.

---

## 3. Instruction Word Format

All instructions have a 16-bit base word, optionally followed by an extra 16-bit word.

### 3.1 Base Instruction Layout

```text
15       11 10     8 7      5 4      2 1      0
+----------+--------+--------+--------+--------+
|  OPCODE  |  MODE  |  RD    |  RS    | UNUSED |
+----------+--------+--------+--------+--------+
  5 bits     3 bits   3 bits   3 bits   2 bits
```

- `OPCODE` (bits 15–11): operation code (up to 32 instructions).
- `MODE` (bits 10–8): addressing mode.
- `RD` (bits 7–5): destination register index (0–3 → `R0–R3`).
- `RS` (bits 4–2): source register index (0–3 → `R0–R3`).
- `UNUSED` (bits 1–0): reserved, must be 0.

Some instructions ignore some of these fields (e.g. jumps may ignore `RD`/`RS`).

### 3.2 Extended Word

Some addressing modes require a second 16-bit word containing:

- Immediate constant (`#imm16`)
- Absolute memory address
- Signed offset (PC-relative or base+offset addressing)

---

## 4. Addressing Modes

The 3-bit `MODE` field selects the addressing mode:

```text
MODE (binary)  Meaning
-------------- --------------------------------------------
000            Register
001            Immediate (extra word)
010            Direct memory (absolute, extra word)
011            Register indirect
100            Register + offset (extra word, signed)
101            PC-relative (extra word, signed)
110–111        Reserved
```

### 4.1 Register Mode (`MODE = 000`)

- Operands are in registers, e.g. `ADD R0, R1`.

### 4.2 Immediate Mode (`MODE = 001`)

- Next word is a 16-bit constant, e.g. `ADD R0, #10`.

### 4.3 Direct Memory Mode (`MODE = 010`)

- Next word is a 16-bit address, e.g. `LOAD R1, [0x2000]`.

### 4.4 Register Indirect Mode (`MODE = 011`)

- Effective address in a register, e.g. `LOAD R0, [R1]`.

### 4.5 Register + Offset Mode (`MODE = 100`)

- Effective address = `R[RS] + sign_extend(offset16)`.

### 4.6 PC-Relative Mode (`MODE = 101`)

- Effective address = `PC + sign_extend(offset16)`.
- Primarily for branches.

---

## 5. Opcode Summary

The CPU supports a small, orthogonal instruction set.

```text
OPCODE  Dec  Mnemonic  Description
------  ---  --------  -----------------------------------------
00000   0    NOP       No operation
00001   1    HALT      Stop execution

00010   2    MOV       Move between registers / imm / memory
00011   3    LOAD      Load from memory into RD
00100   4    STORE     Store from register to memory

00101   5    ADD       RD ← RD + operand
00110   6    SUB       RD ← RD - operand
00111   7    AND       RD ← RD & operand
01000   8    OR        RD ← RD | operand
01001   9    XOR       RD ← RD ^ operand
01010   10   CMP       Compare (like SUB; result discarded, flags only)

01011   11   SHL       Logical shift left
01100   12   SHR       Logical shift right

01101   13   JMP       Unconditional jump
01110   14   JZ        Jump if Z = 1
01111   15   JNZ       Jump if Z = 0
10000   16   JC        Jump if C = 1
10001   17   JNC       Jump if C = 0
10010   18   JN        Jump if N = 1

10011   19   CALL      Call subroutine (push PC; jump)
10100   20   RET       Return from subroutine (pop PC)

10101   21   PUSH      Push register onto stack
10110   22   POP       Pop value from stack into register

10111   23   IN        Read from IO port into RD
11000   24   OUT       Write RS to IO port

11001–11111           Reserved for future use
```

More detailed per-instruction semantics (valid addressing modes, flag updates) can be derived from this table and the CPU reference documentation.

