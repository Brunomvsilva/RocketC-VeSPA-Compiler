# RocketC VeSPA Compiler

This project is a compiler for the **RocketC** language, specifically targeting the **VeSPA** processor architecture.

## Project Evolution
Originally built as a procedural compiler in C, this project is currently undergoing a major architectural refactor to modern **C++17**. It serves as a personal laboratory for implementing advanced software design patterns and modern memory management techniques.

### Core Refactor Objectives
* **Visitor Pattern Implementation**: Decoupling the AST structure from the logic of semantic analysis and code generation.
* **Modern Memory Management**: Utilizing **RAII** and smart pointers (`std::unique_ptr`) to eliminate manual `free()` calls and prevent memory leaks.
* **Type Safety**: Transitioning from a generic `TreeNode` structure to a robust class hierarchy (e.g., `LiteralNode`, `BinaryOpNode`, `FuncDeclNode`).
* **Standard Library Integration**: Replacing custom legacy data structures with the C++ Standard Template Library (STL), such as `std::unordered_map` for Symbol Tables.

---

## Architecture & Design
The refactored version centers around the **Visitor Design Pattern**. This allows the compiler to traverse the Abstract Syntax Tree (AST) multiple times to perform different tasks without modifying the node classes themselves.

![Refactor Class Diagram](Images/RefactorClassDiagram.png)

### Key Components:
* **AST Nodes**: Represented by the `Node` hierarchy. These are "dumb" structures that only store data and "accept" a Visitor.
* **SemanticVisitor**: Responsible for scope analysis, symbol table management (`SymbolTable`), and type checking.
* **ConstFoldingVisitor**: An optimization pass that evaluates constant expressions at compile time.
* **CodeGenVisitor**: Handles the translation of the AST into VeSPA-compatible assembly, interacting with a `RegisterAllocator`.

---

## Directory Structure
* `../OldCompiler`: The legacy procedural C implementation (Reference).
* `./refactor`: The modern C++ codebase.
    * `./AST`: Definition of the new node hierarchy.
    * `./Semantics`: Logic for semantic analysis and symbol tables.
    * `./Bridge`: Integration layer between the legacy Lexer/Parser (Flex/Bison) and the new C++ AST.
