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

	// Print a stack trace 
	for(int i = vm.frame_count-1; i >= 0; --i)
	{
		CallFrame* frame = &vm.frames[i];
		ObjFunction* function = frame->function;

		// The instruction pointer always points to the NEXT instruction
		// to execute, so we subtract 1 here so that we are sitting on the 
		// current instruction.
		size_t instr = frame->ip - function->chunk.code - 1;
		fprintf(stderr, "[line %d] in ", function->chunk.lines[instr]);
		if(function->name == NULL)
			fprintf(stderr, "script\n");
		else
			fprintf(stderr, "%s()\n", function->name->chars);
	}

	reset_stack();
}


// ======== VM stack operations ======== //
static void reset_stack(void)
{
	vm.stack_top = vm.stack;
	vm.frame_count = 0;
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


/*
 * call()
 */
static bool call(ObjFunction* function, int arg_count)
{
	if(arg_count > function->arity)
	{
		runtime_error("Expected %d arguments but got %d.", function->arity, arg_count);
		return false;
	}

	// Make sure we don't overflow the call frame array
	if(vm.frame_count == FRAMES_MAX)
	{
		runtime_error("Stack overflow");
		return false;
	}

	CallFrame* frame = &vm.frames[vm.frame_count++];
	frame->function = function;
	frame->ip = function->chunk.code;
	frame->slots = vm.stack_top - arg_count - 1;

	return true;
}

/*
 * call_value()
 */
static bool call_value(Value callee, int arg_count)
{
	if(IS_OBJ(callee))
	{
		switch(OBJ_TYPE(callee))
		{
			case OBJ_FUNCTION:
				return call(AS_FUNCTION(callee), arg_count);

			default:
				// Non-callable objects 
				break;
		}
	}

	runtime_error("Can only call functions and classes.");
	return false;
}


static InterpResult run(void) 
{
	CallFrame* frame = &vm.frames[vm.frame_count-1];

	// NOTE: why use a macro here? Faster? Because its inlined?
#define READ_BYTE() (*frame->ip++)
#define READ_CONSTANT() (frame->function->chunk.constants.values[READ_BYTE()])
#define READ_STRING() AS_STRING(READ_CONSTANT())
#define READ_SHORT() (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))

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

		disassemble_instr(&frame->function->chunk, (int)(frame->ip - frame->function->chunk.code));
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
				push(frame->slots[slot]);
				break;
			}

			case OP_SET_LOCAL: {
				uint8_t slot = READ_BYTE();
				frame->slots[slot] = peek(0);
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
				frame->ip += offset;
				break;
			}

			case OP_JUMP_IF_FALSE: {
				uint16_t offset = READ_SHORT();
				if(is_falsey(peek(0)))
					frame->ip += offset;
				break;
		    }

			case OP_LOOP: {
				uint16_t offset = READ_SHORT();
				frame->ip -= offset;
				break;
			}

			case OP_CALL: {
				int arg_count = READ_BYTE();
				if(!call_value(peek(arg_count), arg_count))
					return INTERPRET_RUNTIME_ERROR;

				frame = &vm.frames[vm.frame_count-1];
				break;
			}

			case OP_RETURN: {
				Value result = pop();
				vm.frame_count--;
				
				if(vm.frame_count == 0)
				{
					// No more call frames - program is over
					pop();
					return INTERPRET_OK;
				}

				vm.stack_top = frame->slots;
				push(result);

				frame = &vm.frames[vm.frame_count-1];
				break;
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
	ObjFunction* function = compile(source);
	if(function == NULL)
		return INTERPRET_COMPILE_ERROR;

	push(OBJ_VAL(function));
	call_value(OBJ_VAL(function), 0);

	return run();
}


/*
 * DEBUG FUNCTIONS FOR VM
 */

void print_vm_stack(void)
{
	// Save the old stack_top pointer
	Value* old_stack_top = vm.stack_top;


	// Restore the old top pointer
	vm.stack_top = old_stack_top;
}

