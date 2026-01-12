# Toy C-to-MIPS Compiler

## Overview
This project implements a toy compiler that translates a small subset of the C
programming language into MIPS assembly. The compiler demonstrates the full
compilation pipeline, from lexical analysis and parsing to code generation.

The primary goal of this project was to gain hands-on experience with how
high-level language constructs are lowered into assembly and how control flow,
memory, and expressions are represented at the machine level.

---

## Features
- Lexical analysis and tokenization
- Recursive-descent parsing into an abstract syntax tree (AST)
- Support for variables, arithmetic expressions, and control flow
- Code generation targeting MIPS assembly
- Stack-based memory management for local variables

---

## Project Structure
```text
toy-c-compiler/
├── toycc.cpp
├── README.md
├── .gitignore
└── examples/
    ├── fizzbuzz_simple.c
    └── fizzbuzz.asm
```
Build & Run

Compile the compiler
``` bash
g++ -std=c++17 -O2 -o toycc toycc.cpp
```
Generate MIPS assembly
``` bash
./toycc examples/fizzbuzz_simple.c -o examples/fizzbuzz.asm
```
Example
``` C
int main() {
    int i = 1;
    for (i; i <= 100; i = i + 1) {
        if (i % 15 == 0) {
            print_str("FizzBuzz\n");
        } else if (i % 3 == 0) {
            print_str("Fizz\n");
        } else if (i % 5 == 0) {
            print_str("Buzz\n");
        } else {
            print_int(i);
            print_str("\n");
        }
    }
    return 0;
}
```
Systems & Security Relevance

Building a compiler provides insight into how vulnerabilities can emerge from
parsing, memory layout, and control-flow generation. This project strengthens
understanding of:
``` markdown
-Language parsing and AST construction
-Stack-based memory management
-Control-flow translation
-How undefined behavior and memory errors can propagate to assembly
```
Author

**Cade Fair**

Computer Science Student — San Diego State University

Cybersecurity & Systems Programming Focus
