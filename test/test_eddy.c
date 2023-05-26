#include "unity.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef size_t eddy_size_t;

char test_prompt[] = "~test>";
char test_phrase[] = "Test phrase!";
char test_print_buffer[256];
char test_hint_buffer[256];
char test_exec_buffer[256];

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

void print_console(const char* string)
{
	strcpy(test_print_buffer, string);
}

void check_hint(char* cmd_line)
{
    strcpy(test_print_buffer, cmd_line);
}

eddy_retv_t exec_command(const char* cmd_line)
{
    strcpy(test_exec_buffer, cmd_line);
}

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

	result = eddy.destroy(&eddy);

	TEST_ASSERT_EQUAL(result, EDDY_RETV_OK);
}

void test_prompt_printing()
{
	eddy_t eddy;
	eddy_retv_t result;

	result = init_eddy(&eddy);

	TEST_ASSERT_EQUAL(result, EDDY_RETV_OK);

	eddy.set_cli_print_clbk(&eddy, print_console);
	eddy.set_check_hint_clbk(&eddy, check_hint);
	eddy.set_exec_cmd_clbk(&eddy, exec_command);
	eddy.set_prompt(&eddy, test_prompt);

	eddy.show_prompt(&eddy);

	TEST_ASSERT_EQUAL_STRING(test_print_buffer, test_prompt);
}

void test_one_char_printing()
{
	eddy_t eddy;
	eddy_retv_t result;
	char test_char = 'X';
	char expected_string[] = "X";

	result = init_eddy(&eddy);

	TEST_ASSERT_EQUAL(result, EDDY_RETV_OK);

	eddy.set_cli_print_clbk(&eddy, print_console);
	eddy.set_check_hint_clbk(&eddy, check_hint);
	eddy.set_exec_cmd_clbk(&eddy, exec_command);
	eddy.put_char(&eddy, test_char);

	TEST_ASSERT_EQUAL_STRING(test_print_buffer, expected_string);
}

void test_more_chars_printing()
{
	eddy_t eddy;
	eddy_retv_t result;
	char expected_string[] = "X";

	result = init_eddy(&eddy);

	TEST_ASSERT_EQUAL(result, EDDY_RETV_OK);

	eddy.set_cli_print_clbk(&eddy, print_console);
	eddy.set_check_hint_clbk(&eddy, check_hint);
	eddy.set_exec_cmd_clbk(&eddy, exec_command);

	for(int idx; idx < sizeof(test_phrase); idx++) {
		eddy.put_char(&eddy, test_phrase[idx]);
		expected_string[0] = test_phrase[idx];
		TEST_ASSERT_EQUAL_STRING(test_print_buffer, expected_string);
	}
}