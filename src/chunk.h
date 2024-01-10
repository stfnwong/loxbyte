#ifndef __CHUNK_H
#define __CHUNK_H

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
	ValueArray constants;
} Chunk;


// NOTE: is there a future where I want to do a cheap 
// namespacing by prefixing all these with chunk_.* ? 
void init_chunk(Chunk* chunk);
void free_chunk(Chunk* chunk);
void write_chunk(Chunk* chunk, uint8_t data);
int add_constant(Chunk* chunk, Value value);


#endif /*__CHUNK_H*/
