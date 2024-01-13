// Loxbyte Virtual Machine
#ifndef __LOX_VM_H
#define __LOX_VM_H


#include "chunk.h"
#include "value.h"
#include "table.h"


#define STACK_MAX 256


typedef struct {
	Chunk* chunk;
	uint8_t* ip;  // instruction pointer for upcoming instruction
	Value stack[STACK_MAX];
	Value* stack_top;
	Table strings;
	Table globals;
	Obj* objects;		// head of objects linked list
} VM;


typedef enum {
	INTERPRET_OK,
	INTERPRET_COMPILE_ERROR,
	INTERPRET_RUNTIME_ERROR,
} InterpResult;


// Stack manipulation
void  push(Value value);
Value pop(void);
Value peek(int dist);

// Virtual Machine
void init_vm(void);
void free_vm(void);
InterpResult interpret(const char* source);


extern VM vm;

#endif /*__LOX_VM_H*/
