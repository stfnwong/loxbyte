#ifndef __LOX_CHUNK_H
#define __LOX_CHUNK_H

#include "common.h"
#include "value.h"



typedef enum {
	OP_CONSTANT,
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
