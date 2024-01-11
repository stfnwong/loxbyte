#include <stdio.h>

#include "memory.h"
#include "value.h"


bool values_equal(Value a, Value b)
{
	if(a.type != b.type)
		return false;

	switch(a.type)
	{
		case VAL_BOOL:
			return AS_BOOL(a) == AS_BOOL(b);
		case VAL_NIL:
			return true;
		case VAL_NUMBER:
			return AS_NUMBER(a) == AS_NUMBER(b);  // TODO: float compare....
		default:
			return false;			// <- unreachable
	}
}


void init_value_array(ValueArray* array)
{
	array->count = 0;
	array->capacity = 0;
	array->values = NULL;
}

void free_value_array(ValueArray* array)
{
	FREE_ARRAY(Value, array->values, array->capacity);
	init_value_array(array);
}

void write_value_array(ValueArray* array, Value value)
{
	if(array->capacity < array->count + 1)
	{
		int prev_capacity = array->capacity;
		array->capacity = GROW_CAPACITY(prev_capacity);
		array->values = GROW_ARRAY(Value, array->values, prev_capacity, array->capacity);
	}

	array->values[array->count] = value;
	array->count++;
}



void print_value(Value value)
{
	switch(value.type)
	{
		case VAL_BOOL:
			printf(AS_BOOL(value) ? "true" : "false");
			break;
		case VAL_NUMBER:
			printf("%g", AS_NUMBER(value));
			break;
		case VAL_NIL:
			printf("nil");
			break;
	}
}
