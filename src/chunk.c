#include "chunk.h"
#include "memory.h"



/*
 * init_chunk()
 */
void init_chunk(Chunk* chunk)
{
	chunk->count = 0;
	chunk->capacity = 0;
	chunk->code = NULL;
	init_value_array(&chunk->constants);
}


/*
 * free_chunk()
 */
void free_chunk(Chunk* chunk)
{
	FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
	free_value_array(&chunk->constants);
	init_chunk(chunk);
}


/*
 * write_chunk()
 */
void write_chunk(Chunk* chunk, uint8_t data)
{
	if(chunk->capacity < chunk->count + 1)
	{
		int prev_capacity = chunk->capacity;
		chunk->capacity = GROW_CAPACITY(prev_capacity);
		chunk->code = GROW_ARRAY(uint8_t, chunk->code, prev_capacity, chunk->capacity);
	}

	chunk->code[chunk->count] = data;
	chunk->count++;
}



int add_constant(Chunk* chunk, Value value)
{
	write_value_array(&chunk->constants, value);
	return chunk->constants.count - 1;	// return index of value
}
