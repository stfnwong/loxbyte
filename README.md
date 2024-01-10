# LOXBYTE
The bytecode interpreter from `Crafting Interpreters`. This is exactly the implementation
from the book in C99.


## Requirements
- C99 compiler (I use `gcc`).
- `make`.


## Howto
One `Makefile` handles everything. To build all the sources run 

`make all`

Which builds the intepreter binary and places it in the root directory.


## What's implemented
- Simple memory management
- Base of virtual machine with binary operators.
- Start of debugger, stack tracing, disassembler.
- Scanning.
