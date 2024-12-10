# Simple brainfuck interpreter

`bfi` is a simple brainfuck interpreter, created as an experiment in interpreter and compiler design.

`bfi` is made not as a stand-alone application, but as a building block for an eventual compiler. It parses the entire source and creates an Abstract Syntax Tree before interpreting it, which is not needed for an interpreter.

`bfi` is written as a single file of pure C, and depends only on libc.
