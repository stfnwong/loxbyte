// Loxbyte Virtual Machine
#ifndef __LOX_VM_H
#define __LOX_VM_H


#include "chunk.h"


typedef struct {
	Chunk* chunk;
	uint8_t* ip;  // instruction pointer for upcoming instruction
} VM;


typedef enum {
	INTERPRET_OK,
	INTERPRET_COMPILE_ERROR,
	INTERPRET_RUNTIME_ERROR,
} InterpResult;


void init_vm(void);
void free_vm(void);
InterpResult interpret(Chunk* chunk);


#endif /*__LOX_VM_H*/
