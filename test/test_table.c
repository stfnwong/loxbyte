/*
 * Unit test for hash table
 */


#include <assert.h>

#include "table.h"
#include "value.h"
#include "object.h"



void test_insert_value(void)
{
	Table table;

	init_table(&table);

	assert(table.count == 0);

	// Insert a value
	Value test_val = NUMBER_VAL(10.0f);
	const char* key_str = "n";
	// TODO: create a ObjString here for the key

	table_set(&table, AS_STRING(OBJ_VAL(copy_string(key_str, 2))), test_val);

	assert(table.count == 1);

	// Return that value 
	Value out_value;


	// Passing a bogus key returns nothing
}


int main(void)
{
	test_insert_value();

	return 0;
}
