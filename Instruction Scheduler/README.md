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

