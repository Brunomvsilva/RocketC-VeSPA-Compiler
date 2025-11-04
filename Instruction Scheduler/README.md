# Instruction Scheduler

The **Instruction Scheduler** is a compiler optimization stage designed to reduce pipeline hazards and improve execution efficiency on the **VeSPA architecture**.  
It runs **after code generation by the compiler** and **before the assembler**, taking the compiler output and reordering instructions while preserving program correctness.

---

## Motivation

On pipelined CPUs like VeSPA, executing instructions back-to-back may cause:

- **Data Hazards** – when an instruction depends on the result of a previous one.
- **Control Hazards** – caused by branches and jumps.
- **Structural Hazards** – limited hardware resources.

To prevent incorrect execution, the CPU may **stall**, wasting cycles.

The Instruction Scheduler **reorders instructions** to:
- Avoid hazards where possible
- Minimize pipeline stalls
- Preserve program semantics (same final register/memory state)

---

##  Toolchain Execution Flow

C Source Code  
↓  
Compiler (Generates Assembly)  
↓  
Instruction Scheduler ← (this stage)  
↓  
Assembler (Generates Hex/Obj Code)  
↓  
RocketSim / Hardware

---

## What the Scheduler Does

- Parses each instruction and extracts:
  - Mnemonic (ADD, LD, ST, etc.)
  - Source/destination registers
  - Immediate values and type (ALU, Load/Store, Branch)
- Detects hazards:
  - **RAW (Read After Write)**
  - **WAR (Write After Read)**
  - **WAW (Write After Write)**
- Finds **safe replacement instructions** to fill stall positions.
- Produces a **new optimized instruction order**.

---

## Example of Reordering

**Original Program (causes a stall):**

```
LD R1, #2000 ; load memory to R1
ADD R2, R1, R3 ; depends on R1 → stall required
NOP
```

**Optimized:**

```
LD R1, #2000
LDI R4, #5 ; Independent → moved up to avoid NOP
ADD R2, R1, R3
```

More details can be found in the [report](Reports%20and%20Presentation/Compiler_Report.pdf)

---

## Types of Hazards Handled

| Hazard Type | Description | Scheduler Action |
|-------------|-------------|-------------------|
| RAW (Read After Write) | Instruction needs a value not yet written | Delay dependent instruction, or insert unrelated instruction |
| WAR (Write After Read) | A write happens before a previous read | Reorder safely or keep original |
| WAW (Write After Write) | Two writes to the same register without dependencies | Usually safe unless order matters |
| Control (Branches / Jumps) | Program flow may change | Scheduler stops reordering across control boundaries |

---

## Corner Cases Considered

- Branch instructions (`JMP`, `BRA`, `BNE`, `RET`)
- Double stalls from load followed by dependent ALU
- Instructions that affect CPU flags (CMP, ADD, SUB)
- Memory dependencies (ST / LD to same address)

---

## How to Build and Run

This project provides a simple Makefile to compile and run the instruction scheduler.

### Build the Program
Open a terminal inside the project folder and run:

``` bash
make all
```

This compiles all `.c` files and generates the executable:

```bash
instructions_sched
```

### Run the Scheduler
To run the program with the default input and output files (`asm.txt` → `out.asm`), use:

```bash
make run
```


**Input/Output**
- `asm.txt` → assembly input file  
- `out.asm` → output file containing scheduled/optimized instructions  

You can also specify your own input and output files

### Clean Build Files
To remove the compiled executable and generated `.asm` files:

```bash
make clean
```

