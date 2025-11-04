# Simple Makefile for CMake-based project

.PHONY: all clean

# Build everything
all:
	mkdir -p build
	cd build && cmake .. && cmake --build .

# Remove build folder
clean:
	rm -rf build
