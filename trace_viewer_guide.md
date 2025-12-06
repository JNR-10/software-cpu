# Trace Viewer Interface Guide

This document explains all sections of the interactive trace viewer and their functions.

---

## 1. Registers & Flags

**Purpose:** Displays the current state of all CPU registers and status flags.

**Components:**
- **General Purpose Registers (R0-R7):** 8 registers that store temporary data and computation results
- **Program Counter (PC):** Points to the memory address of the current instruction being executed
- **Stack Pointer (SP):** Points to the top of the stack (grows downward from 0x7FFF)
- **Status Flags:**
  - **Z (Zero):** Set when the result of an operation is zero
  - **N (Negative):** Set when the result is negative (MSB = 1)
  - **C (Carry):** Set when arithmetic operations produce a carry/borrow
  - **V (Overflow):** Set when signed arithmetic overflow occurs

**Visual Indicators:**
- Registers that changed in the current cycle are highlighted in green
- Flag values are shown as 0 or 1

---

## 2. Current Instruction

**Purpose:** Shows the instruction being executed in the current cycle.

**Information Displayed:**
- **Opcode:** The operation being performed (e.g., ADD, SUB, LOAD, STORE, CALL, RET)
- **Operands:** The registers or memory addresses involved in the operation
- **Assembly Format:** Human-readable representation of the instruction

**Example:**
```
CALL R1, factorial
```

---

## 3. Stack Memory (Recursion Depth)

**Purpose:** Visualizes the stack's current state and tracks function call depth.

**Key Features:**
- **Stack Depth Indicator:** Shows how many recursion levels deep the program is
- **Stack Usage:** Displays bytes used and number of entries on the stack
- **Stack Entries:** Lists memory addresses and their values
  - **SP (Current Top):** Highlighted entry showing where the stack pointer currently points
  - **Return Addresses:** Addresses in the code segment (0x8000-0xEFFF range) that indicate where to return after a function call
  - **Saved Values:** Function arguments and local variables stored on the stack

**Behavior:**
- Stack grows **downward** from 0x7FFF
- Auto-scrolls to keep the current SP visible as the stack grows
- Empty stack shows: "Stack is empty (SP = 0x7FFF)"

---

## 4. Memory Operations

**Purpose:** Tracks all memory read/write operations during execution.

**Information Shown:**
- **Current Cycle Operations:** Memory writes that occurred in the current cycle
- **Cumulative Memory State:** All memory locations that have been written to, showing their current values
- **Format:** `Address: Value` (e.g., `0x7FED: 0x26`)

**Use Cases:**
- Debug memory corruption issues
- Track how data flows through memory
- Verify correct function argument passing via stack

---

## 5. Controls

**Purpose:** Provides playback controls for stepping through the execution trace.

**Control Buttons:**
- **⏮️ First:** Jump to the first cycle (cycle 0)
- **⏪ Previous:** Step backward one cycle
- **⏩ Next:** Step forward one cycle
- **⏭️ Last:** Jump to the last cycle (end of execution)
- **▶️ Run:** Auto-play through cycles at the selected speed
- **🔄 Reset:** Return to the first cycle

**Speed Control:**
- **0.5x (Slow):** 2000ms per cycle
- **1x (Normal):** 1000ms per cycle
- **2x (Fast):** 500ms per cycle
- **4x (Very Fast):** 250ms per cycle
- **10x (Ultra Fast):** 100ms per cycle
- **Max Speed:** No delay between cycles

---

## 6. Program Code

**Purpose:** Displays the assembly source code with execution tracking.

**Features:**
- **Syntax Highlighting:** Different colors for opcodes, registers, labels, and comments
- **Active Line Highlighting:** The currently executing line is highlighted in yellow
- **Line Numbers:** Shows source line numbers for reference
- **Auto-Scroll:** Automatically scrolls to keep the current instruction visible
- **PC Indicator:** Shows which instruction the Program Counter is pointing to

**Visual Elements:**
- **Labels:** Function and jump target names (e.g., `factorial:`, `loop:`)
- **Comments:** Explanatory text preceded by `;`
- **Instructions:** Assembly mnemonics with operands

---

## 7. Cycle Bar

**Purpose:** Shows execution progress and allows direct navigation to any cycle.

**Components:**
- **Cycle Counter:** Displays current cycle number and total cycles (e.g., "Cycle: 70 / 176")
- **Progress Slider:** Interactive slider to jump to any specific cycle
- **Visual Indicator:** Progress bar showing how far through execution you are

**Usage:**
- Drag the slider to jump to a specific cycle
- Click anywhere on the bar to jump to that position
- Use arrow keys for fine-grained control

---

## 8. Memory Layout

**Purpose:** Visualizes how the executable is organized in the 16-bit address space.

### Memory Segments:

#### 📦 **Data Segment (0x0000 - 0x0FFF)**
- **Size:** 4 KiB
- **Purpose:** Stores global variables, static constants, and string literals
- **Status:** Shows "Active" when PC is reading/writing from this region
- **Note:** Often inactive in programs that use only registers and stack for data

#### 📚 **Stack Segment (0x1000 - 0x7FFF)**
- **Size:** 28 KiB
- **Purpose:** Manages function calls, local variables, return addresses, and recursion
- **Key Metrics:**
  - **SP (Stack Pointer):** Current top of stack address
  - **Stack Used:** Bytes consumed (grows downward from 0x7FFF)
  - **Recursion Depth:** Estimated number of nested function calls
- **Status:** Active during function calls and recursion

#### ⚙️ **Code Segment (0x8000 - 0xEFFF)**
- **Size:** 28 KiB
- **Purpose:** Contains the executable machine code (program instructions)
- **Key Metrics:**
  - **PC (Program Counter):** Address of the current instruction
- **Status:**
  - **🟢 Executing:** Program is actively running
  - **🔴 Halted:** Program has finished (HALT instruction executed)
  - **⚪ Inactive:** PC is not in this segment

#### 🔌 **Memory-Mapped I/O (0xF000 - 0xF0FF)**
- **Size:** 256 bytes
- **Purpose:** Interface for I/O devices, timers, and hardware peripherals
- **Usage:** Reading/writing to these addresses communicates with external devices
- **Status:** Typically inactive unless program uses I/O operations

---

## How to Use the Trace Viewer

1. **Select a Trace:** Click on a trace button (e.g., "Factorial Recursive") from the landing page
2. **Navigate:** Use the cycle slider or control buttons to move through execution
3. **Observe Changes:** Watch registers, flags, stack, and memory update in real-time
4. **Track Execution:** Follow the highlighted line in the Program Code panel
5. **Analyze Behavior:** Use Memory Layout to understand how the program uses different memory regions
6. **Debug Issues:** Check Memory Operations to see all reads/writes

---

## Tips for Effective Debugging

- **Watch the Stack:** Monitor recursion depth to detect infinite recursion or stack overflow
- **Track Register Changes:** Green highlights show which registers were modified
- **Follow the PC:** The Program Code panel shows exactly where execution is
- **Check Flags:** Status flags help understand conditional branches and comparisons
- **Use Slow Speed:** Start with 0.5x or 1x speed to carefully observe behavior
- **Jump to Key Points:** Use the slider to quickly navigate to interesting cycles

---

## Example: Analyzing Recursive Factorial

When viewing the "Factorial Recursive" trace:

1. **Cycle 0-10:** Initial setup, loading arguments
2. **Middle Cycles:** Recursive calls building up the stack
   - Watch Stack Memory grow downward
   - See return addresses being saved
   - Observe recursion depth increasing
3. **Peak Recursion:** Maximum stack depth reached
4. **Return Phase:** Stack unwinding as functions return
   - Stack pointer moves back up
   - Results are computed and returned
5. **Final Cycle:** Program halts, Code Segment shows "🔴 Halted"

---

*This trace viewer provides a complete window into CPU execution, making it easy to understand how programs execute at the machine level.*
