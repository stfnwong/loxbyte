#include <stdio.h>

#include "vm.h"

VM vm;


static InterpResult run(void) 
{
	// NOTE: why use a macro here? Faster? Because its inlined?
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

	for(;;)
	{
		uint8_t instr = READ_BYTE();
		switch(instr)
		{
			case OP_CONSTANT: {
				Value constant = READ_CONSTANT();
				print_value(constant);
				fprintf(stdout, "\n");
				break;
			}
			case OP_RETURN:
				return INTERPRET_OK;
		}
	}
#undef READ_BYTE
#undef READ_CONSTANT
}


void init_vm(void) {}

void free_vm(void) {}


InterpResult interpret(Chunk* chunk)
{
	vm.chunk = chunk;
	vm.ip = vm.chunk->code;

	return run();
}

