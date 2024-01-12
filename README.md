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

## Grammar
Its the same grammar as before (since its the same language). These are the productions
implemented so far.

### 

### Statements
The statement grammar at the time of writing is 

- `program -> declaration | eof;`
- `declaration -> statement;`
- `statement -> expr_stmt | print_stmt`
- `expr_stmt -> expression ";"`
- `print_stmt -> "print" expression ";"`


## What's implemented
- Simple memory management
- Base of virtual machine with binary operators, string concatenation.
- REPL that implements 
- Start of debugger, stack tracing, disassembler.
- Scanning, compilation. Implements Pratt parser. 
- Hash Table.
