# Simple brainfuck interpreter

`bfi` is a simple brainfuck interpreter, created as an experiment in interpreter and compiler design.

`bfi` is intended not as a stand-alone application, but as a building block for an eventual compiler. It parses the entire source and creates an Abstract Syntax Tree before interpreting said AST, as a compiler would.

`bfi` is written as a single file of pure C, and depends only on libc.
