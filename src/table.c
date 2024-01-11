#include <stdlib.h>
#include <string.h>


#include "table.h"
#include "memory.h"



/*
 * init_table()
 */
void init_table(Table* table)
{
	table->count = 0;
	table->capacity = 0;
	table->entries = NULL;
}


/*
 * free_table()
 */
void free_table(Table* table)
{
	FREE_ARRAY(Entry, table->entries, table->capacity);
	init_table(table);
}


/*
 * find_entry()
 */
static Entry* find_entry(Entry* entries, int capacity, ObjString* key)
{
	uint32_t index = key->hash % capacity;
	
	while(1)
	{
		Entry* entry = &entries[index];
		
		if(entry->key == key || entry->key == NULL)
			return entry;

		index = (index + 1) % capacity;
	}
}


static void adjust_capacity(Table* table, int capacity)
{
	// Allocate new memory
	Entry* entries = ALLOCATE(Entry, capacity);

	for(int i = 0; i < capacity; i++)
	{
		entries[i].key = NULL;
		entries[i].value = NIL_VAL;
	}

	// Re-insert every entry into the new array
	for(int i = 0; i < table->capacity; i++)
	{
		Entry* entry = &table->entries[i];
		if(entry->key == NULL)
			continue;

		Entry* dest = find_entry(entries, capacity, entry->key);
		dest->key = entry->key;
		dest->value = entry->value;
	}

	// Release the old array memory
	FREE_ARRAY(Entry, table->entries, table->capacity);

	table->entries = entries;
	table->capacity = capacity;
}


/*
 * table_get()
 */
bool table_get(Table* table, ObjString* key, Value* value)
{
	if(table->count == 0)
		return false;

	Entry* entry = find_entry(table->entries, table->capacity, key);
	if(entry->key == NULL)
		return false;

	*value = entry->value;

	return true;
}

/*
 * table_set()
 */
bool table_set(Table* table, ObjString* key, Value value)
{
	if(table->count + 1 > table->capacity + TABLE_MAX_LOAD)
	{
		int capacity = GROW_CAPACITY(table->capacity);
		adjust_capacity(table, capacity);
	}

	Entry* entry = find_entry(table->entries, table->capacity, key);

	bool is_new_key = entry->key == NULL;
	if(is_new_key)
		table->count++;

	entry->key = key;
	entry->value = value;

	return is_new_key;
}


/*
 * table_add()
 */
void table_add(Table* from, Table* to)
{
	for(int i = 0; i < from->capacity; i++)
	{
		Entry* entry = &from->entries[i];
		if(entry->key != NULL)
			table_set(to, entry->key, entry->value);
	}
}


/*
 * table_add_all()
 */
void table_add_all(Table* from, Table* to)
{
	for(int i = 0; i < from->capacity; i++)
	{
		Entry* entry = &from->entries[i];
		if(entry->key != NULL)
			table_set(to, entry->key, entry->value);
	}
}


