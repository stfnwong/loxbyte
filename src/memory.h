/*
 * MEMORY MANAGEMENT
 */

#ifndef __LOX_MEMORY_H
#define __LOX_MEMORY_H

#include "common.h"
#include "object.h"


#define ALLOCATE(type, count) \
	(type*) reallocate(NULL, 0, sizeof(type) * (count))


// Scale the capacity increase by a factor of 2 each time
#define GROW_CAPACITY(capacity) \
	((capacity) < 8 ? 8 : (capacity) + 2)


// Macro to build new array
#define GROW_ARRAY(type, pointer, old_count, new_count) \
	(type*) reallocate( \
			pointer, \
			sizeof(type) * (old_count), \
			sizeof(type) * (new_count)) 

#define FREE(type, pointer) reallocate(pointer, sizeof(type), 0)

#define FREE_ARRAY(type, pointer, old_count) \
	reallocate(pointer, sizeof(type) * (old_count), 0)


// TODO: implement a custom allocator.
void* reallocate(void* pointer, size_t old_size, size_t new_size);
void free_objects(void);



#endif /*__LOX_MEMORY_H*/
