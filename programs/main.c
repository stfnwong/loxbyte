#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"



Chunk simple_expr(void)
{
	Chunk chunk;
	int constant;
	int lineno = 123;

	init_chunk(&chunk);
	
	// Hand-compile a constant instruction 
	constant = add_constant(&chunk, 1.2f);
	write_chunk(&chunk, OP_CONSTANT, lineno);
	write_chunk(&chunk, constant, lineno);
	write_chunk(&chunk, OP_NEGATE, lineno);
	write_chunk(&chunk, OP_RETURN, lineno);

	return chunk;
}


Chunk multipart_expr(void)
{	
	Chunk chunk;
	int constant;
	int lineno = 234;

	init_chunk(&chunk);
	
	// Hand-compile a constant instruction 
	constant = add_constant(&chunk, 1.2f);
	write_chunk(&chunk, OP_CONSTANT, lineno);
	write_chunk(&chunk, constant, lineno);

	// Here we hand implement the expression 
	// -((1.2 + 3.4) / (5.6))

	constant = add_constant(&chunk, 3.4);
	write_chunk(&chunk, OP_CONSTANT, lineno);
	write_chunk(&chunk, constant, lineno);

	write_chunk(&chunk, OP_ADD, lineno);

	constant = add_constant(&chunk, 5.6);
	write_chunk(&chunk, OP_CONSTANT, lineno);
	write_chunk(&chunk, constant, lineno);

	write_chunk(&chunk, OP_DIV, lineno);
	write_chunk(&chunk, OP_NEGATE, lineno);


	write_chunk(&chunk, OP_RETURN, lineno);

	return chunk;
}


int main(int argc, char *argv[])
{
	init_vm();

	Chunk chunk;

	chunk = simple_expr();
	disassemble_chunk(&chunk, "simple_expr");
	interpret(&chunk);

	chunk = multipart_expr();
	disassemble_chunk(&chunk, "multipart_expr");
	interpret(&chunk);

	// Cleanup
	free_vm();
	free_chunk(&chunk);

	return 0;
}
