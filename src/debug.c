#include <stdio.h>

#include "debug.h"



// Instruction util functions go here 
static int simple_instr(const char* name, int offset)
{
	fprintf(stdout, "%s\n", name);
	return offset + 1;
}


/*
 * disassemble_chunk()
 */
void disassemble_chunk(Chunk* chunk, const char* name)
{
	fprintf(stdout, "== %s ==\n", name);

	for(int offset = 0; offset < chunk->count;)
		offset = disassemble_instr(chunk, offset);
}


/*
 * disassemble_instr()
 */
int disassemble_instr(Chunk* chunk, int offset)
{
	fprintf(stdout, "%04X ", offset);

	uint8_t instr = chunk->code[offset];
	switch(instr)
	{
		case OP_RETURN:
			return simple_instr("OP_RETURN", offset);
		default:
			fprintf(stdout, "Unknown opcode %d\n", instr);
			return offset + 1;
	}

	return 0;		// <- should be unreachable
}
