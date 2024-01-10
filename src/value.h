#ifndef __VALUE_H
#define __VALUE_H

#include "common.h"

// For now all values are floats
typedef double Value;		


// We hold non-immediate values in a constant pool. This is implemented as an array 
// of values. 


typedef struct {
	int capacity;
	int count;
	Value* values;
} ValueArray;


void init_value_array(ValueArray* array);
void free_value_array(ValueArray* array);
void write_value_array(ValueArray* array, Value value);
void print_value(Value value);



#endif /*__VALUE_H*/
