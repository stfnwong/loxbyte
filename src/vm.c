#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "vm.h"
#include "debug.h"


VM vm;


static InterpResult run(void) 
{
	// NOTE: why use a macro here? Faster? Because its inlined?
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

#define BINARY_OP(op) \
	do { \
		double b = pop(); \
		double a = pop(); \
		push(a op b); \
	} while(false)

	for(;;)
	{
#ifdef DEBUG_TRACE_EXECUTION
		fprintf(stdout, "      ");
		for(Value* slot = vm.stack; slot < vm.stack_top; slot++)
		{
			fprintf(stdout, "[");
			print_value(*slot);
			fprintf(stdout, "]");
		}
		fprintf(stdout, "\n");

		disassemble_instr(vm.chunk, (int)(vm.ip - vm.chunk->code));
#endif /*DEBUG_TRACE_EXECUTION*/

		uint8_t instr = READ_BYTE();
		switch(instr)
		{
			case OP_CONSTANT: {
				Value constant = READ_CONSTANT();
				push(constant);
				print_value(constant);
				fprintf(stdout, "\n");
				break;
			}
			case OP_ADD: BINARY_OP(+); break;
			case OP_SUB: BINARY_OP(-); break;
			case OP_MUL: BINARY_OP(*); break;
			case OP_DIV: BINARY_OP(/); break;
			case OP_NEGATE: {
				push(-pop());
				break;
			}
			case OP_RETURN: {
				print_value(pop());
				fprintf(stdout, "\n");
				return INTERPRET_OK;
			}
		}
	}

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}


// ======== VM stack operations ======== //
static void reset_stack(void)
{
	vm.stack_top = vm.stack;
}

void push(Value value)
{
	*vm.stack_top = value;
	vm.stack_top++;
}

Value pop(void)
{
	vm.stack_top--;
	return *vm.stack_top;
}


void init_vm(void)
{
	reset_stack();
}


void free_vm(void) {}


InterpResult interpret(const char* source)
{
	Chunk chunk;

	init_chunk(&chunk);

	if(!compile(source, &chunk))
	{
		free_chunk(&chunk);
		return INTERPRET_COMPILE_ERROR;
	}

	vm.chunk = &chunk;
	vm.ip = vm.chunk->code;

	InterpResult res = run();

	free_chunk(&chunk);

	return INTERPRET_OK;
}

