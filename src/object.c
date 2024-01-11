#include <stdio.h>
#include <string.h>

#include "object.h"
#include "memory.h"
#include "vm.h"



#define ALLOCATE_OBJ(type, obj_type) \
	(type*) allocate_object(sizeof(type), obj_type)


static Obj* allocate_object(size_t size, ObjType type)
{
	Obj* object = (Obj*) reallocate(NULL, 0, size);
	object->type = type;
	object->next = vm.objects;
	vm.objects = object;

	return object;
}


static ObjString* allocate_string(char* chars, int length)
{
	ObjString* str = ALLOCATE_OBJ(ObjString, OBJ_STRING);
	str->length = length;
	str->chars = chars;

	return str;
}


/*
 * copy_string()
 * Copy constructor for an ObjString
 */
ObjString* copy_string(const char* chars, int length)
{
	char* heap_chars = ALLOCATE(char, length + 1);
	memcpy(heap_chars, chars, length);
	heap_chars[length] = '\0';
	
	return allocate_string(heap_chars, length);
}


/*
 * take_string()
 * Move constructor for a ObjString.
 */
ObjString* take_string(const char* chars, int length)
{
	return allocate_string(chars, length);
}


void print_object(Value value)
{
	switch(OBJ_TYPE(value))
	{
		case OBJ_STRING:
			fprintf(stdout, "%s", AS_CSTRING(value));
			break;
	}
}
