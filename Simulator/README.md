# RocketSim – VeSPA Simulator

RocketSim is a virtual runtime environment for the VeSPA SoC. It executes VeSPA machine code on an x86 host and offers debugging facilities (breakpoints, register/memory dumps) plus peripheral mocking (e.g., UART input). This avoids slow deploy cycles to FPGA hardware and the limitations of on-target debugging.

---

## Features

- Runs VeSPA machine code from the intermediate hex `.txt` format
- Interactive menu-based CLI
- Peripheral mocking (UART RX prompt) for user input during execution

---

## How to Build

### Using the provided Makefile

    make all      # creates build/, runs CMake, compiles RocketSim
    make clean    # removes build/

### Manual CMake build (alternative)

    mkdir -p build
    cd build
    cmake ..
    cmake --build .

The executable is produced at: `build/RocketSim`.

---

## Run the Example (Fibonacci)

Run the simulator with the provided example program:

    ./build/RocketSim testCode.txt

When prompted with “Waiting on UART RX value…”, type a hex value for N (e.g., `A` for decimal 10) and press Enter.


### Example Output (after Fibonacci for N = 0x0A)

    HALT found! Stopping CPU!
    1. Free Run
    2. Goto next breakpoint
    3. Set breakpoint
    4. Dump registers
    5. Dump memory
    6. Exit
    4
    R0 -> 55
    R1 -> 34
    R2 -> 89

---

## CLI (Command Line Interface)

When RocketSim starts, it shows:

1. Free Run
2. Goto next breakpoint
3. Set breakpoint
4. Dump registers
5. Dump memory
6. Exit

| Option | Action               | Description                                                                 | Syntax / Example                      |
|--------|----------------------|-----------------------------------------------------------------------------|---------------------------------------|
| **1**  | Free Run             | Runs continuously until **HALT** or a peripheral (e.g., UART) requests input. | –                                     |
| **2**  | Goto next breakpoint | Runs until a **breakpoint** or **HALT** is reached.                         | –                                     |
| **3**  | Set breakpoint       | Adds a breakpoint at a **code address** (hex, byte address).               | `3 <AddressHex>` → `3 0000001C`       |
| **4**  | Dump registers       | Prints all CPU registers and status flags.                                  | –                                     |
| **5**  | Dump memory          | Prints memory contents between two **hex** addresses.                       | `5 <StartHex> <EndHex>` → `5 0000 0040` |
| **6**  | Exit                 | Quits the simulator.                                                        | –                                     |

---

## UART Mock (Input)

When the program reads from the UART RX address, RocketSim pauses and prints:

    Waiting on UART RX value...

- Enter a value in hexadecimal (e.g., `A` → decimal 10) and press Enter.
- Execution resumes immediately.

---


## Input File Format (Intermediate Hex)

RocketSim loads instructions from a text file where each line encodes one 32-bit instruction into four bytes at a given code address:

    @<Address> <Byte3> <Byte2> <Byte1> <Byte0>

All fields are hexadecimal (no `0x` prefix).

| Field        | Meaning                                  |
|--------------|------------------------------------------|
| `@Address`   | Code memory byte address (hex)           |
| `Byte3..0`   | Instruction bytes (MSB to LSB)           |
| Format       | Hexadecimal values without `0x` prefix   |

Example:

    @0004 58 40 00 00
    @0008 58 80 00 00
    @000C 50 C0 04 0B

These lines are typically produced by the VeSPA toolchain/assembler and map directly to instruction bytes stored in code memory.

---

## Notes

- Up to 16 breakpoints are supported.
- Breakpoint addresses are in **bytes** (code memory address space).
- If execution appears to do nothing, verify the input file path and that the lines match the specified format.
