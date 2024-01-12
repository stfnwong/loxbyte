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

### Statements
The statement grammar at the time of writing is 

- `program -> declaration | eof;`
- `declaration -> class_decl | func_decl | var_decl | statement;`
- `class_decl -> "class" IDENTIFIER "{" function* "}";`
- `func_decl -> "func" function;`
- `function -> IDENTIFIER "(" parameters? ")" block;`
- `parameters -> IDENTIFIER ( "," IDENTIFIER* ")";`
- `var_decl -> "var" IDENTIFIER { "=" expression }? ";"`
- `statement -> expr_stmt | for_stmt | if_stmt | print_stmt | return_stmt | while_stmt | block; `
- `expr_stmt -> expression ";"`
- `for_stmt -> "(" ( var_decl | expr_stmt | ";" ) expression? ";" expression ";" ")" statement;`  (Desugared to while loop)
- `if_stmt -> "(" expression ")" statment "else" statement ")"?;`
- `return_stmt -> "return" expression? ";"`
- `print_stmt -> "print" expression ";"`
- `while_stmt -> "while" "(" expression ")" statement;`
- `block -> "{" declaration* "};"`


## What's implemented
- Simple memory management
- Base of virtual machine with binary operators, string concatenation.
- REPL that implements 
- Start of debugger, stack tracing, disassembler.
- Scanning, compilation. Implements Pratt parser. 
- Hash Table.
