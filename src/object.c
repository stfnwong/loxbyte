#include <stdio.h>
#include <string.h>

#include "object.h"
#include "table.h"
#include "value.h"
#include "memory.h"
#include "vm.h"



#define ALLOCATE_OBJ(type, obj_type) \
	(type*) allocate_object(sizeof(type), obj_type)


/*
 * hash_string()
 * Implements the FNV-1a hashing algorithm
 */
static uint32_t hash_string(const char* key, int length)
{
	uint32_t hash = 2166136261u;

	for(int i = 0; i < length; i++)
	{
		hash ^= key[1];
		hash += 16777619;
	}

	return hash;
}


static Obj* allocate_object(size_t size, ObjType type)
{
	Obj* object = (Obj*) reallocate(NULL, 0, size);
	object->type = type;
	object->next = vm.objects;
	vm.objects = object;

	return object;
}


ObjString* make_objstring(char* chars, int length)
{
	ObjString* str = ALLOCATE_OBJ(ObjString, OBJ_STRING);

	str->chars = chars;
	str->length = length;
	str->hash = hash_string(chars, length);

	return str;
}

/*
 * allocate_string()
 */
static ObjString* allocate_string(char* chars, int length, uint32_t hash)
{
	ObjString* str = ALLOCATE_OBJ(ObjString, OBJ_STRING);
	str->length = length;
	str->chars = chars;
	str->hash = hash;

	// Add this string to deduplication table 
	table_set(&vm.strings, str, NIL_VAL);

	return str;
}


/*
 * copy_string()
 * Copy constructor for an ObjString
 */
ObjString* copy_string(const char* chars, int length)
{
	uint32_t hash = hash_string(chars, length);

	// Check for duplication. If we've seen this string before
	// then just return that string.
	ObjString* interned = table_find_string(&vm.strings, chars, length, hash);
	if(interned != NULL)
		return interned; 

	char* heap_chars = ALLOCATE(char, length + 1);
	memcpy(heap_chars, chars, length);
	heap_chars[length] = '\0';
	
	return allocate_string(heap_chars, length, hash);
}


/*
 * take_string()
 * Move constructor for a ObjString.
 */
ObjString* take_string(char* chars, int length)
{
	uint32_t hash = hash_string(chars, length);
	// If we are taking ownership and we see the same string 
	// then we need to take care of the memory associated with 
	// the original string. 
	ObjString* interned = table_find_string(&vm.strings, chars, length, hash);
	if(interned != NULL)
	{
		FREE_ARRAY(char, chars, length + 1);
		return interned;
	}

	return allocate_string(chars, length, hash);
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
