/*
 * Unit test for hash table
 */

#include <stdlib.h>
#include <check.h>


#include "table.h"
#include "value.h"
#include "object.h"
#include "util.h"


START_TEST(test_insert_value)
{
	Table table;
	bool ret;

	init_table(&table);

	ck_assert(table.count == 0);
	//ck_assert_int_eq(table.count, 0);

	// Insert a value
	Value test_val = NUMBER_VAL(10.0f);
	ObjString* key = make_objstring("n", 1);
	ret = table_set(&table, key, test_val);
	ck_assert(ret == true);

	ck_assert(table.count == 1);

	Value out_value;
	ret = table_get(&table, key, &out_value);
	ck_assert(ret == true);

	ck_assert(out_value.type == VAL_NUMBER);
	ck_assert(float_equal(AS_NUMBER(out_value), 10.0f));

	// Passing a bogus key returns nothing
	ObjString* bogus_key = make_objstring("junk", 4);
	ret = table_get(&table, bogus_key, &out_value);
	ck_assert(ret == false);
}
END_TEST


Suite* table_suite(void)
{
	Suite* s;

	s = suite_create("Hash Table");

	// Test insert values 
	TCase* tc_insert = tcase_create("Insert Values");
	tcase_add_test(tc_insert, test_insert_value);
	suite_add_tcase(s, tc_insert);

	return s;
}


int main(void)
{
	int num_failed;

	Suite* s;
	SRunner* sr;

	s = table_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	num_failed = srunner_ntests_failed(sr);

	srunner_free(sr);

	return num_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
