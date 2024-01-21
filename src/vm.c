#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "common.h"
#include "compiler.h"
#include "vm.h"
#include "memory.h"
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


static void concatenate(void)
{
	ObjString* bstr = AS_STRING(pop());
	ObjString* astr = AS_STRING(pop());

	int length = astr->length + bstr->length;
	char* chars = ALLOCATE(char, length + 1);
	memcpy(chars, astr->chars, astr->length);
	memcpy(chars + astr->length, bstr->chars, bstr->length);
	chars[length] = '\0';

	ObjString* result = take_string(chars, length);
	push(OBJ_VAL(result));
}



static InterpResult run(void) 
{
	// NOTE: why use a macro here? Faster? Because its inlined?
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define READ_STRING() AS_STRING(READ_CONSTANT())
#define READ_SHORT() (vm.ip += 2, (uint16_t)((vm.ip[-2] << 8) | vm.ip[-1]))

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

			case OP_POP:
				pop();
				break;

			case OP_DEFINE_GLOBAL: {
				ObjString* name = READ_STRING();
				table_set(&vm.globals, name, peek(0));
				pop();
				break;

			}

			case OP_GET_GLOBAL: {
				ObjString* name = READ_STRING();
				Value value;

				if(!table_get(&vm.globals, name, &value))
				{
					runtime_error("Undefined variable '%s'.", name->chars);
					return INTERPRET_RUNTIME_ERROR;
				}

				push(value);
				break;
			}

			case OP_SET_GLOBAL: {
				ObjString* name = READ_STRING();

				// Variable declaration in Lox is not implicit,
				// so setting a value to a name that has not 
				// been declared is an error.
				if(table_set(&vm.globals, name, peek(0)))
				{
					table_delete(&vm.globals, name);
					runtime_error("Undefined variable '%s'.", name->chars);
					return INTERPRET_RUNTIME_ERROR;
				}

				break;
			}

			case OP_GET_LOCAL: {
				uint8_t slot = READ_BYTE();
				push(vm.stack[slot]);
				break;
			}

			case OP_SET_LOCAL: {
				uint8_t slot = READ_BYTE();
				vm.stack[slot] = peek(0);
				break;
			}

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

			// Add either two numbers or concat two strings
			case OP_ADD: {
				if(IS_STR(peek(0)) && IS_STR(peek(1)))
					concatenate();
				else if(IS_NUMBER(peek(0)) && IS_NUMBER(peek(1)))
				{
					double b = AS_NUMBER(pop());
					double a = AS_NUMBER(pop());
					push(NUMBER_VAL(a + b));
				}
				else
				{
					runtime_error("Operands must be numbers or strings");
					return INTERPRET_RUNTIME_ERROR;
				}
				break;
			}
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

			case OP_PRINT: {
				print_value(pop());
				printf("\n");
				break;
			}

			case OP_JUMP: {
				uint16_t offset = READ_SHORT();
				vm.ip += offset;
				break;
			}

			case OP_JUMP_IF_FALSE: {
				uint16_t offset = READ_SHORT();
				if(is_falsey(peek(0)))
					vm.ip += offset;
				break;
		    }

			case OP_RETURN: {
				return INTERPRET_OK;
			}
		}
	}

#undef READ_BYTE
#undef READ_CONSTANT
#undef READ_SHORT
#undef READ_STRING
#undef BINARY_OP
}


void init_vm(void)
{
	reset_stack();
	vm.objects = NULL;
	init_table(&vm.strings);
	init_table(&vm.globals);
}


void free_vm(void)
{
	free_table(&vm.strings);
	free_table(&vm.globals);
	free_objects();
}


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

