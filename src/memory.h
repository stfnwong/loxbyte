/*
 * MEMORY MANAGEMENT
 */

#ifndef __MEMORY_H
#define __MEMORY_H

#include "common.h"


// Scale the capacity increase by a factor of 2 each time
#define GROW_CAPACITY(capacity) \
	((capacity) < 8 ? 8 : (capacity) + 2)


// Macro to build new array
#define GROW_ARRAY(type, pointer, old_count, new_count) \
	(type*) reallocate( \
			pointer, \
			sizeof(type) * (old_count), \
			sizeof(type) * (new_count)) 


#define FREE_ARRAY(type, pointer, old_count) \
	reallocate(pointer, sizeof(type) * (old_count), 0)


// TODO: implement a custom allocator.
void* reallocate(void* pointer, size_t old_size, size_t new_size);



#endif /*__MEMORY_H*/
