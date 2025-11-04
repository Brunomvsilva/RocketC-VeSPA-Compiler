## RocketSim - VeSPA Simulator

RocketSim is a virtual runtime environment for the VeSPA SoC. It executes VeSPA **machine code** on an x86 host and offers basic debugging facilities (breakpoints, register/memory dumps) plus **peripheral mocking** (e.g., UART input). This avoids slow deploy cycles to FPGA hardware and the limitations of on-target debugging.

## Features

- Run VeSPA machine code (intermediate hex `.txt` format).
- Menu CLI
- Peripheral mocking (UART RX prompt).

---

## How to Build

### Using the provided Makefile

    make all      # creates build/, runs CMake, compiles RocketSim
    make clean    # removes build/

### Run testCode - Fibonacci Sequence

``` bash
./build/RocketSim testCode.txt
```
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
|--------|------------------------|-----------------------------------------------------------------------------|----------------------------------------|
| **1**  | Free Run              | Runs the program continuously until **HALT** or a peripheral (e.g., UART) requests user input. | – |
| **2**  | Goto next breakpoint  | Runs the program until hitting a **breakpoint** or **HALT**.               | – |
| **3**  | Set breakpoint        | Adds a breakpoint at a specific **code address (hex, byte address)**.     | `3 <AddressHex>` → `3 0000001C` |
| **4**  | Dump registers        | Displays the values of **all CPU registers and status flags**.             | – |
| **5**  | Dump memory           | Prints memory contents between two **hexadecimal addresses**.              | `5 <StartHex> <EndHex>` → `5 0000 0040` |
| **6**  | Exit                  | Exits the simulator immediately.                                           | – |


---

## Example: Fibonacci simulation

1) Run:
    ```
    ./build/RocketSim testCode.txt
    ```
3) Select `1` (Free Run). When prompted:

    ```
    Waiting on UART RX value...
    ```
   enter a hex value for N, e.g., `A` (10 decimal).

4) Execution continues until HALT. Then you can inspect the result (e.g., option `4` to dump registers).

Example register dump after Fibonacci(10):

    HALT found! Stopping CPU!
    1. Free Run
    2. Goto next breakpoint
    3. Set breakpoint
    4. Dump registers
    5. Dump memory
    6. Exit
    4
    R0 -> 55     (Fibonacci(10))
    R1 -> 34
    R2 -> 89

---
