#include "unity.h"
#include <stdbool.h>
#include <stdlib.h>

typedef size_t eddy_size_t;

void* eddy_malloc(eddy_size_t size)
{
	static bool first_time = true;

	if(first_time) {
		first_time = false;
		return NULL;
	} else {
		return malloc(size);
	}
}

#include "eddy.h"

void setUp(void) {}

void tearDown(void) {}

void* false_maloc(size_t size, int num_calls){
	return NULL;
}

void test_init_eddy_malloc_err()
{
	eddy_t eddy;
	eddy_retv_t result;

	result = init_eddy(&eddy);

	TEST_ASSERT_EQUAL(result, EDDY_RETV_ERR);
}

void test_init_eddy_ok()
{
	eddy_t eddy;
	eddy_retv_t result;

	result = init_eddy(&eddy);

	TEST_ASSERT_EQUAL(result, EDDY_RETV_OK);
}

