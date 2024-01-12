#ifndef __LOX_TABLE_H
#define __LOX_TABLE_H


#include "common.h"
#include "value.h"


#define TABLE_MAX_LOAD 0.75



/*
 * Entry
 * A single entry in a Hash Table
 */
typedef struct {
	ObjString* key;
	Value value;
} Entry;



/*
 * Table
 * A Hash Table
 */
typedef struct {
	int count;
	int capacity;
	Entry* entries;
} Table;


void init_table(Table* table);
void free_table(Table* table);

bool table_get(Table* table, ObjString* key, Value* value);
bool table_set(Table* table, ObjString* key, Value value);
void table_add(Table* from, Table* to);
void table_add_all(Table* from, Table* to);
ObjString* table_find_string(Table* table, const char* chars, int length, uint32_t hash);
bool table_delete(Table* table, ObjString* key);


#endif /*__LOX_TABLE_H*/
