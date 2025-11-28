
# Memory Map

This document describes the 16-bit address space layout of the Software CPU.

The CPU uses 16-bit addresses, giving a total address space of:

- `0x0000–0xFFFF` (64 KiB)

Program code, data, stack, and memory-mapped I/O share this unified address space.

---

## 1. Global Layout

Proposed layout:

```text
Address Range      Description
-----------------  -------------------------------------------
0x0000–0x7FFF      RAM (32 KiB) – data + stack
0x8000–0xEFFF      Program area (code + static data)
0xF000–0xF0FF      Memory-mapped I/O
0xF100–0xFFFF      Reserved (e.g., reset vector, ROM)
```

The emulator will enforce any special behavior associated with these regions (e.g. I/O side effects).

---

## 2. RAM Region (0x0000–0x7FFF)

- Size: 32 KiB
- Usage:
  - General-purpose variables and data structures.
  - **Stack** for subroutine calls and local data.

### 2.1 Stack

- The stack lives in RAM and **grows downward** (toward lower addresses).
- A typical initial value:
  - `SP = 0x7FFF`
- Instructions such as `PUSH`, `POP`, `CALL`, and `RET` operate on this stack using the `SP` register.

---

## 3. Program Area (0x8000–0xEFFF)

- Used for program code and static data.
- The emulator can load assembled programs into this region.
- A typical convention is to place the entry point at:
  - `PC = 0x8000` at reset/startup.

The precise layout within this region (code vs. read-only data) is determined by the assembler/linker or loader.

---

## 4. Memory-Mapped I/O (0xF000–0xF0FF)

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

## 5. Reserved / Vectors (0xF100–0xFFFF)

The top of memory is reserved for future extensions such as:

- Interrupt and exception vectors
- Reset vector (initial `PC` value)
- ROM or fixed configuration data

One possible convention is to store a reset vector at:

- `0xFFFC–0xFFFD`: 16-bit start address loaded into `PC` at reset

For Phase 1 and initial emulator implementations, this region may simply be unused or treated as read-only.

