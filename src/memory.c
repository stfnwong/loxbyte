#include <stdlib.h>

#include "memory.h"
#include "vm.h"



/*
 * reallocate()
 */
void* reallocate(void* pointer, size_t old_size, size_t new_size)
{
	if(new_size == 0) {
		free(pointer);
		return NULL;
	}

	void* result = realloc(pointer, new_size);

	return result;
}


/*
 * free_object()
 * Free an Obj's memory.
 */
void free_object(Obj* object)
{
	switch(object->type)
	{
		case OBJ_STRING: {
			ObjString* str = (ObjString*) object;
			FREE_ARRAY(char, str->chars, str->length + 1);
			FREE(ObjString, object);
			break;
		}
		case OBJ_FUNCTION: {
			ObjFunction* function = (ObjFunction*) object;
			free_chunk(&function->chunk);
			FREE(ObjFunction, object);
			break;
		}
	}
}


/*
 * free_objects()
 * Free all objects attached to the VM
 */
void free_objects(void)
{
	Obj* object = vm.objects;
	
	while(object != NULL)
	{
		Obj* next = object->next;
		free_object(object);
		object = next;
	}
}
