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
	Entry* tombstone = NULL;
	
	while(1)
	{
		Entry* entry = &entries[index];

		// While following the probe sequence if we see a 
		// tombstone note it and continue.

		if(entry->key == NULL)
		{
			if(IS_NIL(entry->value))
			{
				// Found an empty entry
				return tombstone != NULL ? tombstone : entry;
			}
			else 
			{
				// Found a tombstone
				if(tombstone == NULL)
					tombstone = entry;
			}
		}
		else if(entry->key == key)
			return entry;		// this is the entry we are looking for

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

	// Re-insert every entry into the new array. We don't
	// bother copying tombstones. This means we need to 
	// recalculate the count from scratch.
	table->count = 0;
	for(int i = 0; i < table->capacity; i++)
	{
		Entry* entry = &table->entries[i];
		if(entry->key == NULL)
			continue;

		Entry* dest = find_entry(entries, capacity, entry->key);
		dest->key = entry->key;
		dest->value = entry->value;
		table->count++;
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
	// Only increment key if the new entry is going into a 
	// completely new bucket (ie: not a tombstone).
	if(is_new_key && IS_NIL(entry->value))
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



/*
 * table_find_string()
 */
ObjString* table_find_string(Table* table, const char* chars, int length, uint32_t hash)
{
	if(table->count == 0)
		return NULL;

	uint32_t index = hash % table->capacity;

	while(1)
	{
		Entry* entry = &table->entries[index];

		if(entry->key == NULL)
		{
			// If we find any empty non-tombstone entry then stop
			if(IS_NIL(entry->value))
				return NULL;
		}
		else if((entry->key->length == length) && 
				(entry->key->hash == hash) &&
				(memcmp(entry->key->chars, chars, length) == 0))
			return entry->key;  // <- this is the key we want

		index = (index + 1) % table->capacity;
	}
}


/*
 * table_delete()
 */
bool table_delete(Table* table, ObjString* key)
{
	if(table->count == 0)
		return false;

	// Find the entry 
	Entry* entry = find_entry(table->entries, table->capacity, key);
	if(entry->key == NULL)
		return false;

	// Place a tombstone on the deleted entry 
	entry->key = NULL;
	entry->value = BOOL_VAL(true);

	return true;
}
