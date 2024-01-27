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

- `expression -> assignment;`


### Statements
The statement grammar at the time of writing is 

- `program -> declaration | eof;`
- `declaration -> statement;`
- `statement -> expr_stmt | print_stmt | block`
- `expr_stmt -> expression ";"`
- `print_stmt -> "print" expression ";"`
- `block_stmt -> "{" declaration "}";`




## What's implemented
- Simple memory management
- Base of virtual machine with binary operators, string concatenation.
- REPL that implements 
- Start of debugger, stack tracing, disassembler.
- Scanning, compilation. Implements Pratt parser. 
- Hash Table.


## Things to implement
- Run-length encoding of `get_line()`.
- Optimize global variable slot table so that we don't add new entries for the same function.
