#ifndef __LOX_VALUE_H
#define __LOX_VALUE_H

#include "common.h"


typedef enum {
	VAL_BOOL,
	VAL_NIL,
	VAL_NUMBER,
} ValueType;


typedef struct {
	ValueType type;
	union {
		bool boolean;
		double number;
	} as;
} Value;


#define IS_BOOL(value)    ((value).type == VAL_BOOL)
#define IS_NUMBER(value)  ((value).type == VAL_NUMBER)
#define IS_NIL(value)     ((value).type == VAL_NIL)

#define AS_BOOL(value)    ((value).as.boolean)
#define AS_NUMBER(value)  ((value).as.number)

#define BOOL_VAL(value)   ((Value) {VAL_BOOL, {.boolean = value}})
#define NUMBER_VAL(value) ((Value) {VAL_NUMBER, {.number = value}})
#define NIL_VAL           ((Value) {VAL_NIL, {.number = 0}})


// We hold non-immediate values in a constant pool. This is implemented as an array 
// of values. 


typedef struct {
	int capacity;
	int count;
	Value* values;
} ValueArray;


bool values_equal(Value a, Value b);

void init_value_array(ValueArray* array);
void free_value_array(ValueArray* array);
void write_value_array(ValueArray* array, Value value);
void print_value(Value value);



#endif /*__LOX_VALUE_H*/
