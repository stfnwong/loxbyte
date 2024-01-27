#ifndef __LOX_CHUNK_H
#define __LOX_CHUNK_H

#include "common.h"
#include "value.h"


/*
 * VM Opcodes
 */
typedef enum {
	OP_CONSTANT,
	OP_NIL,
	OP_TRUE,
	OP_FALSE,
	OP_POP,
	OP_DEFINE_GLOBAL,
	OP_GET_GLOBAL,
	OP_SET_GLOBAL,
	OP_GET_LOCAL,
	OP_SET_LOCAL,
	OP_EQUAL,
	OP_GREATER,
	OP_LESS,
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	OP_NOT,
	OP_NEGATE,
	OP_PRINT,
	OP_JUMP,
	OP_JUMP_IF_FALSE,
	OP_LOOP,
	OP_CALL,
	OP_RETURN,
} OpCode;


typedef struct {
	int count;
	int capacity;
	uint8_t* code;
	int* lines;    // NOTE: is the order important for alignment?
	ValueArray constants;
} Chunk;


// NOTE: is there a future where I want to do a cheap 
// namespacing by prefixing all these with chunk_.* ? 
void init_chunk(Chunk* chunk);
void free_chunk(Chunk* chunk);
void write_chunk(Chunk* chunk, uint8_t data, int line);
int add_constant(Chunk* chunk, Value value);

// TODO: implement a get_line() that does RLE on the line number


#endif /*__LOX_CHUNK_H*/
