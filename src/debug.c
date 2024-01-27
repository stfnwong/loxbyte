#include <stdio.h>

#include "debug.h"


// ======== INSTRUCTION UTIL FUNCTIONS ======== //

/*
 * simple_instr()
 */
static int simple_instr(const char* name, int offset)
{
	fprintf(stdout, "%s\n", name);
	return offset + 1;
}

/*
 * const_instr()
 */
static int const_instr(const char* name, Chunk* chunk, int offset)
{
	uint8_t constant = chunk->code[offset + 1];
	fprintf(stdout, "%-16s %4d '", name, constant);
	print_value(chunk->constants.values[constant]);
	fprintf(stdout, "'\n");

	return offset + 2;
}

/*
 * byte_instr()
 */
static int byte_instr(const char* name, Chunk* chunk, int offset)
{
	uint8_t slot = chunk->code[offset + 1];
	fprintf(stdout, "%-16s %4d\n", name, slot);
	
	return offset + 2;
}


/*
 * jump_instr()
 */
static int jump_instr(const char* name, int sign, Chunk* chunk, int offset)
{
	uint16_t jump = (uint16_t) (chunk->code[offset+1] << 8);
	jump |= chunk->code[offset+2];

	fprintf(stdout, "%-16s %4d -> %d\n", name, offset, offset +3 + sign * jump);

	return offset + 3;
}


/*
 * disassemble_chunk()
 */
void disassemble_chunk(Chunk* chunk, const char* name)
{
	fprintf(stdout, "==== %s ====\n", name);
	fprintf(stdout, "Offset  line  instr\n");

	for(int offset = 0; offset < chunk->count;)
		offset = disassemble_instr(chunk, offset);
}


/*
 * disassemble_instr()
 */
int disassemble_instr(Chunk* chunk, int offset)
{
	fprintf(stdout, "%06X ", offset);

	// Show the line number of the instruction 
	if(offset > 0 && chunk->lines[offset] == chunk->lines[offset-1])
		fprintf(stdout, "   |  ");
	else
		fprintf(stdout, "%4d  ", chunk->lines[offset]);

	uint8_t instr = chunk->code[offset];
	switch(instr)
	{
		case OP_RETURN:
			return simple_instr("OP_RETURN", offset);
		case OP_NIL:
			return simple_instr("OP_NIL", offset);
		case OP_TRUE:
			return simple_instr("OP_TRUE", offset);
		case OP_FALSE:
			return simple_instr("OP_FALSE", offset);
		case OP_POP:
			return simple_instr("OP_POP", offset);
		case OP_DEFINE_GLOBAL:
			return const_instr("OP_DEFINE_GLOBAL", chunk, offset);
		case OP_GET_GLOBAL:
			return const_instr("OP_GET_GLOBAL", chunk, offset);
		case OP_SET_GLOBAL:
			return const_instr("OP_SET_GLOBAL", chunk, offset);
		case OP_GET_LOCAL:
			return byte_instr("OP_GET_LOCAL", chunk, offset);
		case OP_SET_LOCAL:
			return byte_instr("OP_SET_LOCAL", chunk, offset);
		case OP_EQUAL:
			return simple_instr("OP_EQUAL", offset);
		case OP_GREATER:
			return simple_instr("OP_GREATER", offset);
		case OP_LESS:
			return simple_instr("OP_LESS", offset);
		case OP_ADD:
			return simple_instr("OP_ADD", offset);
		case OP_SUB:
			return simple_instr("OP_SUB", offset);
		case OP_MUL:
			return simple_instr("OP_MUL", offset);
		case OP_DIV:
			return simple_instr("OP_DIV", offset);
		case OP_NOT:
			return simple_instr("OP_NOT", offset);
		case OP_NEGATE:
			return simple_instr("OP_NEGATE", offset);
		case OP_PRINT:
			return simple_instr("OP_PRINT", offset);
		case OP_JUMP:
			return jump_instr("OP_JUMP", 1, chunk, offset);
		case OP_JUMP_IF_FALSE:
			return jump_instr("OP_JUMP_IF_FALSE", 1, chunk, offset);
		case OP_LOOP:
			return jump_instr("OP_LOOP", -1, chunk, offset);
		case OP_CALL:
			return byte_instr("OP_CALL", chunk, offset);
		case OP_CONSTANT:
			return const_instr("OP_CONSTANT", chunk, offset);
		default:
			fprintf(stdout, "Unknown opcode %d\n", instr);
			return offset + 1;
	}

	return 0;		// <- should be unreachable
}
