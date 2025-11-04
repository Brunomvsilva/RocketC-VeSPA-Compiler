SHELL := /bin/bash


OUT := instructions_sched

CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -O2

SRC := $(wildcard *.c)
HDR := $(wildcard *.h)

.PHONY: all run clean clean-all

all: $(OUT)

$(OUT): $(SRC) $(HDR)
	$(CC) $(CFLAGS) $(SRC) -o "$@"

run: $(OUT)
	./"$(OUT)" asm.txt out.asm

clean:
	rm -f "$(OUT)" *.o
	rm -f optimized.asm

