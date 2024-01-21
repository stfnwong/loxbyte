/*
 * OBJECT
 */

#ifndef __LOX_OBJECT_H
#define __LOX_OBJECT_H

#include "common.h"
#include "value.h"


#define OBJ_TYPE(value)   (AS_OBJ(value)->type)

#define IS_STR(value)     is_obj_type(value, OBJ_STRING)

#define AS_STRING(value)  ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)


typedef enum {
	OBJ_STRING,
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

void print_object(Value value);

static inline bool is_obj_type(Value value, ObjType type)
{
	return IS_OBJ(value) && AS_OBJ(value)->type == type;
}



#endif /*__LOX_OBJECT_H*/
