
# Microarchitecture

This document describes the internal organization (microarchitecture) of the 16-bit Software CPU, including:

- Internal registers and main blocks
- Logical CPU schematic (conceptual)
- Fetch–decode–execute cycle

---

## 1. Main Components

The CPU consists of the following major blocks:

- **Control Unit**
  - Finite-state machine that sequences the fetch, decode, and execute phases.

- **Register File**
  - Programmer-visible registers:
    - `R0–R3` (general-purpose, 16-bit)
    - `SP` (Stack Pointer, 16-bit)
    - `FLAGS` (8-bit; uses bits `Z`, `N`, `C`, `V`)

- **ALU (Arithmetic Logic Unit)**
  - Performs arithmetic and logical operations (`ADD`, `SUB`, `AND`, `OR`, `XOR`, `CMP`, `SHL`, `SHR`).
  - Produces and updates flags (`Z`, `N`, `C`, `V`).

- **Instruction Register (`IR`)**
  - Holds the current 16-bit instruction word being executed.

- **Program Counter (`PC`)**
  - Holds the address of the next instruction word to fetch.

- **Memory Address Register (`MAR`)**
  - Holds the address for the next memory access.

- **Memory Data Register (`MDR`)**
  - Buffers data read from or written to memory.

- **System Memory**
  - Unified 16-bit address space for code, data, stack, and memory-mapped I/O.

An optional temporary register (`TMP`) may be used internally by the implementation.

---

## 2. Logical Schematic (Conceptual)

The following ASCII diagram summarizes the primary data paths:

```text
          +---------------------------+
          |         CONTROL          |
          |   (FSM / microcode)     |
          +------------+------------+
                       |
                       v
+----------+    +------+-----+    +----------+
|   PC     |--->|   MAR     |--->|  MEMORY  |
+----------+    +------+-----+    +----------+
                      |  ^
                      v  |
                   +--------+
                   |  MDR   |
                   +--------+
                       |
                       v
                    +------+
                    |  IR  |
                    +------+
                       |
        +--------------+--------------+
        |                             |
        v                             v
+----------------+            +----------------+
|  REGISTER FILE |<---------->|      ALU       |
| R0 R1 R2 R3    |            | ADD/SUB/AND.. |
| SP, FLAGS      |            | Flag logic     |
+----------------+            +----------------+
         ^                              |
         |                              v
         +--------------------------> FLAGS
```

Key points:

- The **Control Unit** orchestrates when each register drives or latches the shared buses.
- `PC` feeds `MAR` to perform instruction fetches from memory.
- `IR` decodes into control signals driving the ALU and register file.
- The ALU reads and writes the register file and updates the `FLAGS` register.

---

## 3. Internal Registers and Buses

- `PC` → instruction address source during fetch.
- `MAR` → address source for all memory reads/writes.
- `MDR` → buffer between memory and internal data paths.
- `IR` → holds current instruction; its fields drive control logic.
- Register file (`R0–R3`, `SP`) → general computation and addressing.
- `FLAGS` → stores condition codes used by branch instructions.

Buses:

- **Address bus**: primarily driven by `MAR` into memory.
- **Data bus**: connects memory, `MDR`, register file, and ALU as needed.

The implementation in software can model these as simple variables and function calls, but the conceptual hardware organization follows this structure.

---

## 4. Fetch–Decode–Execute Cycle

The CPU repeatedly performs a fetch–decode–execute cycle.

### 4.1 Fetch Phase

1. `MAR ← PC`
2. `MDR ← MEM[MAR]`   (read 16-bit instruction word)
3. `IR ← MDR`
4. `PC ← PC + 2`      (advance to next instruction word)

### 4.2 Decode Phase

The control unit examines fields in `IR`:

- `OPCODE = IR[15:11]`
- `MODE   = IR[10:8]`
- `RD     = IR[7:5]`
- `RS     = IR[4:2]`

Based on these, it determines:

- Whether an extra word must be fetched (immediate, address, offset).
- Which registers provide operands to the ALU.
- Which register or memory location receives the result.
- How and whether to update `PC` and flags.

### 4.3 Execute Phase (Examples)

#### 4.3.1 `ADD` in Register Mode (`MODE = 000`)

1. `A ← R[RD]`, `B ← R[RS]`
2. `RESULT ← A + B` via ALU
3. `R[RD] ← RESULT`
4. Update `Z`, `N`, `C`, `V` based on `RESULT`.

#### 4.3.2 `LOAD RD, [addr]` in Direct Mode (`MODE = 010`)

1. Fetch extra word for address:
   - `MAR ← PC`
   - `MDR ← MEM[MAR]` (address word)
   - `PC ← PC + 2`
2. `MAR ← MDR` (effective address)
3. `MDR ← MEM[MAR]` (read data)
4. `R[RD] ← MDR`
5. Optionally update `Z` and `N` based on loaded value.

#### 4.3.3 `JZ` with PC-Relative Mode (`MODE = 101`)

1. Fetch offset word:
   - `offset ← MEM[PC]`
   - `PC ← PC + 2`
2. If `Z == 1`:
   - `PC ← PC + sign_extend(offset)`
3. Else:
   - Leave `PC` unchanged beyond the increment in step 1 (branch not taken).

---

## 5. Notes for Emulator Implementation

- The emulator can model each phase (fetch, decode, execute) as separate steps or within a single cycle function.
- Internal registers (`PC`, `IR`, `MAR`, `MDR`, `FLAGS`, `R0–R3`, `SP`) should be explicitly represented in the CPU state.
- Memory accesses should honor the memory map (RAM, program area, memory-mapped I/O, reserved regions).

