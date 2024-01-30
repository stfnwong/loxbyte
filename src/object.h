/*
 * OBJECT
 */

#ifndef __LOX_OBJECT_H
#define __LOX_OBJECT_H

#include "common.h"
#include "chunk.h"
#include "value.h"


#define OBJ_TYPE(value)    (AS_OBJ(value)->type)

#define IS_STR(value)      is_obj_type(value, OBJ_STRING)
#define IS_FUNCTION(value) is_obj_type(value, OBJ_FUNCTION)
#define IS_NATIVE(value)   is_obj_type(value, OBJ_NATIVE)

#define AS_STRING(value)   ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value)  (((ObjString*)AS_OBJ(value))->chars)
#define AS_FUNCTION(value) ((ObjFunction*)AS_OBJ(value))
#define AS_NATIVE(value)   (((ObjNative*)AS_OBJ(value))->function)


typedef enum {
	OBJ_STRING,
	OBJ_FUNCTION,
	OBJ_NATIVE,
} ObjType;


/*
 * Generic object structure
 */
struct Obj {
	ObjType type;
	struct Obj* next;
};


/*
 * String object specialization
 */
struct ObjString {
	Obj obj;
	int length;
	char* chars;
	uint32_t hash;
};

ObjString* make_objstring(char* chars, int length);
ObjString* copy_string(const char* chars, int length);
ObjString* take_string(char* chars, int length);


// ==== Regular Functions ===== /

/*
 * Function
 */
typedef struct {
	Obj obj;
	int arity;
	Chunk chunk;
	ObjString* name;
} ObjFunction;


ObjFunction* new_function(void);

// ==== Native Functions ===== //
typedef Value (*NativeFn)(int arg_count, Value* args);

typedef struct {
	Obj obj;				// header
	NativeFn function;		// pointer to C function that implements behaviour
} ObjNative;


ObjNative* new_native(NativeFn function);


// Other junk
void print_object(Value value);

static inline bool is_obj_type(Value value, ObjType type)
{
	return IS_OBJ(value) && AS_OBJ(value)->type == type;
}



#endif /*__LOX_OBJECT_H*/
