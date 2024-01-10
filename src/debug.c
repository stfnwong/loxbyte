#include <stdio.h>

#include "debug.h"


// ======== INSTRUCTION UTIL FUNCTIONS ======== //

static int simple_instr(const char* name, int offset)
{
	fprintf(stdout, "%s\n", name);
	return offset + 1;
}

static int const_instr(const char* name, Chunk* chunk, int offset)
{
	uint8_t constant = chunk->code[offset + 1];
	fprintf(stdout, "%-16s %4d '", name, constant);
	print_value(chunk->constants.values[constant]);
	fprintf(stdout, "'\n");

	return offset + 2;
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
		fprintf(stdout, "  |   ");
	else
		fprintf(stdout, "%4d  ", chunk->lines[offset]);

	uint8_t instr = chunk->code[offset];
	switch(instr)
	{
		case OP_RETURN:
			return simple_instr("OP_RETURN", offset);
		case OP_CONSTANT:
			return const_instr("OP_CONSTANT", chunk, offset);
		default:
			fprintf(stdout, "Unknown opcode %d\n", instr);
			return offset + 1;
	}

	return 0;		// <- should be unreachable
}
