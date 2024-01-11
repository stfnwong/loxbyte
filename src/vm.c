#include <stdio.h>
#include <stdarg.h>

#include "common.h"
#include "compiler.h"
#include "vm.h"
#include "debug.h"


VM vm;

static void reset_stack(void);


static void runtime_error(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fputs("\n", stderr);

	size_t instr = vm.ip - vm.chunk->code - 1;
	int line = vm.chunk->lines[instr];
	fprintf(stderr, "[line %d] in script \n", line);

	reset_stack();
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

Value peek(int dist)
{
	return vm.stack_top[-1 - dist];
}


static bool is_falsey(Value value)
{
	return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}



static InterpResult run(void) 
{
	// NOTE: why use a macro here? Faster? Because its inlined?
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

#define BINARY_OP(value_type, op) \
	do { \
		if(!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {\
			runtime_error("Operands must be numbers"); \
			return INTERPRET_RUNTIME_ERROR; \
		} \
		double b = AS_NUMBER(pop()); \
		double a = AS_NUMBER(pop()); \
		push(value_type(a op b)); \
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
			case OP_NIL:
				push(NIL_VAL);
				break;
			case OP_TRUE:
				push(BOOL_VAL(true));
				break;
			case OP_FALSE:
				push(BOOL_VAL(false));
				break;

			case OP_EQUAL: {
				Value b = pop();
				Value a = pop();
				push(BOOL_VAL(values_equal(a, b)));
				break;
			}

			case OP_GREATER:
				BINARY_OP(BOOL_VAL, >); break;
			case OP_LESS:
				BINARY_OP(BOOL_VAL, <); break;

			case OP_ADD: BINARY_OP(NUMBER_VAL, +); break;
			case OP_SUB: BINARY_OP(NUMBER_VAL, -); break;
			case OP_MUL: BINARY_OP(NUMBER_VAL, *); break;
			case OP_DIV: BINARY_OP(NUMBER_VAL, /); break;
			case OP_NOT:
				push(BOOL_VAL(is_falsey(pop())));
				break;
			case OP_NEGATE: {
				if(!IS_NUMBER(peek(0))) {
					runtime_error("Operand must be a number");
					return INTERPRET_RUNTIME_ERROR;
				}

				push(NUMBER_VAL(-AS_NUMBER(pop())));
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

