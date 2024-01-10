#include "common.h"
#include "chunk.h"
#include "debug.h"


int main(int argc, char *argv[])
{
	Chunk chunk;

	init_chunk(&chunk);
	
	// Hand-compile a constant instruction 
	int constant = add_constant(&chunk, 1.2f);
	write_chunk(&chunk, OP_CONSTANT);
	write_chunk(&chunk, constant);
	write_chunk(&chunk, OP_RETURN);

	disassemble_chunk(&chunk, "test_chunk");

	free_chunk(&chunk);

	return 0;
}
