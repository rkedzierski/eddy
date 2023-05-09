/**
 * @file eddy.c
 * @author Rafał Kędzierski (rafal.kedzierski@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2023-04-22
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "eddy.h"
#include "eddy_config.h"

#include <string.h>
#include <stdio.h>
#include <ctype.h>


#define EDDY_NULL 0
/**
 * @{ \name Escape character codes
 */
#define VT100_BS_CODE  0x08
#define VT100_ESC_CODE 0x1B
#define VT100_DEL_CODE 0x7F

#define VT100_BACKSPACE           "\x08"
#define VT100_ESC                 "\x1b"
#define VT100_CLEAR_SCREEN_DOWN   VT100_ESC "[J"
#define VT100_CLEAR_SCREEN_UP     VT100_ESC "[1J"
#define VT100_CLEAR_ENTIRE_SCREEN VT100_ESC "[2J"
#define VT100_CLEAR_LINE_RIGHT    VT100_ESC "[K"
#define VT100_CLEAR_LINE_LEFT     VT100_ESC "[1K"
#define VT100_CLEAR_ENTIRE_LINE   VT100_ESC "[2K"
#define VT100_MOVE_CURSOR_UP      VT100_ESC "[A"
#define VT100_MOVE_CURSOR_UP_N    VT100_ESC "[%uA"
#define VT100_MOVE_CURSOR_DOWN    VT100_ESC "[B"
#define VT100_MOVE_CURSOR_DOWN_N  VT100_ESC "[%uB"
#define VT100_MOVE_CURSOR_RIGHT   VT100_ESC "[C"
#define VT100_MOVE_CURSOR_RIGHT_N VT100_ESC "[%uC"
#define VT100_MOVE_CURSOR_LEFT    VT100_ESC "[D"
#define VT100_MOVE_CURSOR_LEFT_N  VT100_ESC "[%uD"
#define VT100_SAVE_CURSOR_POS     VT100_ESC "[s"
#define VT100_RESTORE_CURSOR_POS  VT100_ESC "[u"
#define VT100_INSERT              VT100_ESC "[2~"
#define VT100_DELETE              VT100_ESC "[3~"
#define VT100_PAGE_UP             VT100_ESC "[5~"
#define VT100_PAGE_DOWN           VT100_ESC "[6~"
#define VT100_HOME                VT100_ESC "[H"
#define VT100_END                 VT100_ESC "[F"
#define VT100_F1                  VT100_ESC "OP"
#define VT100_F2                  VT100_ESC "OQ"
#define VT100_F3                  VT100_ESC "OR"
#define VT100_F4                  VT100_ESC "OS"
#define VT100_F5                  VT100_ESC "[15~"
#define VT100_F6                  VT100_ESC "[17~"
#define VT100_F7                  VT100_ESC "[18~"
#define VT100_F8                  VT100_ESC "[19~"
#define VT100_F9                  VT100_ESC "[20~"
#define VT100_F10                 VT100_ESC "[21~"
#define VT100_F11                 VT100_ESC "[23~"
#define VT100_F12                 VT100_ESC "[24~"
/**
 * @}
 */

/**
 * @{ \name Definitions of internal beffer lenghts.
 */
#define EDDY_MAX_ESC_SEQ_LEN		7	/**< Lenght of escape sequence buffer. */
#define EDDY_MAX_PROMPT_LEN			8	/**< Lenght of prompt buffer. */
/**
 * @}
 */

/**
 * @brief Structure contain del and backspace codes
 * 
 */
typedef struct eddy_keys_codes_s {
	char bs_key;		/**< Back space key code. */
	char del_key;		/**< Delete key code. */
} eddy_keys_codes_t;

/**
 * @brief Private internal context of library
 * 
 */
typedef struct eddy_ctx_s {
	char line_buffer[EDDY_MAX_LINE_BUFF_LEN];	/**< Edited line buffer. */
	unsigned int line_len;						/**< Number of entered characters. */
	unsigned int line_pos;						/**< Cursor position in buffer. */
	char esc_seq[EDDY_MAX_ESC_SEQ_LEN+1];		/**< Buffer on escape sequence. */
	unsigned int esc_seq_len;					/**< Number of characters in escape sequence buffer. */
	char prompt[EDDY_MAX_PROMPT_LEN];			/**< Buffer with prompt. */
	eddy_keys_codes_t keys_codes;				/**< Structure with back space and delete codes. */

	eddy_cli_print_clbk cli_print_clbk;			/**< Pointer on terminal printing function. */
	eddy_log_print_clbk log_print_clbk;			/**< Pointer on logs printing function. */
	eddy_check_hint_clbk check_hint_clbk;		/**< Pointer on check and print hints function. */
	eddy_exec_cmd_clbk exec_cmd_clbk;			/**< Pointer on command execution function */
} eddy_ctx_t;

/**
 * @{ \name API implementation functions.
*/
eddy_retv_t eddy_put_char_impl(eddy_p self, char c);
eddy_retv_t eddy_set_cli_print_impl(eddy_p self, eddy_cli_print_clbk cli_print_clbk);
eddy_retv_t eddy_set_log_print_impl(eddy_p self, eddy_log_print_clbk log_print_clbk);
eddy_retv_t eddy_set_check_hint_impl(eddy_p self, eddy_check_hint_clbk check_hint_clbk);
eddy_retv_t eddy_set_exec_cmd_impl(eddy_p self, eddy_exec_cmd_clbk exec_cmd_clbk);
eddy_retv_t eddy_show_prompt_impl(eddy_p self);
/**
 * @}
 */
/**
 * @{ \name Private functions declarations.
 */
eddy_retv_t eddy_proces_insert_char(eddy_p self, char c);
eddy_retv_t eddy_process_cursor_left(eddy_p self);
eddy_retv_t eddy_process_cursor_right(eddy_p self);
eddy_retv_t eddy_process_check_hint(eddy_p self, char* cmd_line);
eddy_retv_t eddy_process_exec_cmd(eddy_p self, const char* cmd_line);
eddy_retv_t eddy_process_bs_key(eddy_p self);
eddy_retv_t eddy_process_del_key(eddy_p self);
eddy_retv_t eddy_print(eddy_p self, const char* buffer);
eddy_retv_t eddy_put(eddy_p self, char chr);
/**
 * @}
 */

eddy_retv_t init_eddy(eddy_p self)
{
	if(self == EDDY_NULL) {
		return EDDY_RETV_ERR;
	}

	self->ctx = eddy_malloc(sizeof(self->ctx));

	if(self->ctx == EDDY_NULL) {
		return EDDY_RETV_ERR;
	}

    self->put_char = eddy_put_char_impl;
	self->set_cli_print_clbk = eddy_set_cli_print_impl;
	self->set_log_print_clbk = eddy_set_log_print_impl;
	self->set_check_hint_clbk = eddy_set_check_hint_impl;
	self->set_exec_cmd_clbk = eddy_set_exec_cmd_impl;
	self->show_prompt = eddy_show_prompt_impl;

	self->ctx->keys_codes.bs_key = VT100_DEL_CODE; /* VT100_BS_CODE; */
	self->ctx->keys_codes.del_key = VT100_BS_CODE;

	self->ctx->line_len = 0;
	self->ctx->line_pos = 0;
	self->ctx->esc_seq_len = 0;

	self->ctx->prompt[0] = '>';
	self->ctx->prompt[1] = '\0';

	self->ctx->cli_print_clbk = EDDY_NULL;
	self->ctx->log_print_clbk = EDDY_NULL;
	self->ctx->check_hint_clbk = EDDY_NULL;
	self->ctx->exec_cmd_clbk = EDDY_NULL;

	return EDDY_RETV_OK;
}

/**
 * @brief Implementation of api set_cli_print_clbk function.
 * 
 * Function to set callback on terminal printing function.
 * 
 * @param self Pointer on library context.
 * @param cli_print_clbk Pointer to print function.
 * @return eddy_retv_t Error code: EDDY_RETV_OK if succes or EDDY_RETV_ERR if error.
 */
eddy_retv_t eddy_set_cli_print_impl(eddy_p self, eddy_cli_print_clbk cli_print_clbk)
{
	if(self == EDDY_NULL || cli_print_clbk == EDDY_NULL) {
		return EDDY_RETV_ERR;
	}

	self->ctx->cli_print_clbk = cli_print_clbk;

	return EDDY_RETV_OK;
}

/**
 * @brief Implementation of api set_log_print_clbk function.
 * 
 * @param self Pointer on library context.
 * @param log_print_clbk Pointer to print function.
 * @return eddy_retv_t Error code: EDDY_RETV_OK if succes or EDDY_RETV_ERR if error.
 */
eddy_retv_t eddy_set_log_print_impl(eddy_p self, eddy_log_print_clbk log_print_clbk)
{
	if(self == EDDY_NULL || log_print_clbk == EDDY_NULL) {
		return EDDY_RETV_ERR;
	}

	self->ctx->log_print_clbk = log_print_clbk;

	return EDDY_RETV_OK;
}

/**
 * @brief Implementation of api set_check_hint_clbk function.
 * 
 * @param self Pointer on library context.
 * @param check_hint_clbk Pointer to check and print hint function.
 * @return eddy_retv_t Error code: EDDY_RETV_OK if succes or EDDY_RETV_ERR if error.
 */
eddy_retv_t eddy_set_check_hint_impl(eddy_p self, eddy_check_hint_clbk check_hint_clbk)
{
	if(self == EDDY_NULL || check_hint_clbk == EDDY_NULL) {
		return EDDY_RETV_ERR;
	}

	self->ctx->check_hint_clbk = check_hint_clbk;

	return EDDY_RETV_OK;
}

/**
 * @brief Implementation of api set_exec_cmd_clbk function.
 * 
 * @param self Pointer on library context.
 * @param exec_cmd_clbk Pointer to execute command function.
 * @return eddy_retv_t Error code: EDDY_RETV_OK if succes or EDDY_RETV_ERR if error.
 */
eddy_retv_t eddy_set_exec_cmd_impl(eddy_p self, eddy_exec_cmd_clbk exec_cmd_clbk)
{
	if(self == EDDY_NULL || exec_cmd_clbk == EDDY_NULL) {
		return EDDY_RETV_ERR;
	}

	self->ctx->exec_cmd_clbk = exec_cmd_clbk;

	return EDDY_RETV_OK;
}

/**
 * @brief Implementation of api put_char function.
 * 
 * @param self Pointer on library context.
 * @param c Character passed from terminal.
 * @return eddy_retv_t Error code: EDDY_RETV_OK if succes or EDDY_RETV_ERR if error.
 */
eddy_retv_t eddy_put_char_impl(eddy_p self, char c)
{
	eddy_retv_t error = EDDY_RETV_OK;

	if(self == EDDY_NULL) {
		return EDDY_RETV_ERR;
	}

	if(c == self->ctx->keys_codes.bs_key) {
		error = eddy_process_bs_key(self);
	} else if(c == self->ctx->keys_codes.del_key) {
		error = eddy_process_del_key(self);
	} else if(self->ctx->esc_seq_len > 0) {
		if(self->ctx->esc_seq_len < EDDY_MAX_ESC_SEQ_LEN) {
			self->ctx->esc_seq[self->ctx->esc_seq_len++] = c;
			self->ctx->esc_seq[self->ctx->esc_seq_len] = '\0';
		}

		if(isalpha(c) || c == '~') {	/* seq end */
			if(!strcmp(self->ctx->esc_seq, VT100_DELETE)) {
			error = eddy_process_del_key(self);
			} else if(!strcmp(self->ctx->esc_seq, VT100_MOVE_CURSOR_LEFT)) {
			error = eddy_process_cursor_left(self);
			} else if(!strcmp(self->ctx->esc_seq, VT100_MOVE_CURSOR_RIGHT)) {
			error = eddy_process_cursor_right(self);
			} else if(!strcmp(self->ctx->esc_seq, VT100_MOVE_CURSOR_UP)) {

			} else if(!strcmp(self->ctx->esc_seq, VT100_MOVE_CURSOR_DOWN)) {

			} else {
			//Unknown escape sequence
				printf("DEBUG: unknown esc seq: %s\n", self->ctx->esc_seq + 1);
				eddy_print(self, self->ctx->esc_seq);
			}
			self->ctx->esc_seq_len = 0;
		}
	} else if(c == VT100_ESC_CODE) {	/* seq start */
		self->ctx->esc_seq[0] = c;
		self->ctx->esc_seq_len = 1;
	} else if(c == '\t') {
		error = eddy_process_check_hint(self, self->ctx->line_buffer);
	} else if(c == '\n') {
		error = eddy_process_exec_cmd(self, self->ctx->line_buffer);
	} else {
		error = eddy_proces_insert_char(self, c);
	}

	return error;
}

/**
 * @brief Fonction shows prompt
 * 
 * @param self Pointer on library context.
 * @return eddy_retv_t Error code: EDDY_RETV_OK if succes or EDDY_RETV_ERR if error.
 */
eddy_retv_t eddy_show_prompt_impl(eddy_p self)
{
	return eddy_print(self, self->ctx->prompt);
}

/**
 * @brief Insert char into line buffer.
 * 
 * @param self Pointer on library context.
 * @param c Character to insertion.
 * @return eddy_retv_t Error code: EDDY_RETV_OK if succes or EDDY_RETV_ERR if error.
 */
eddy_retv_t eddy_proces_insert_char(eddy_p self, char c)
{
	eddy_retv_t error = EDDY_RETV_OK;

	if(self->ctx->line_len < (EDDY_MAX_LINE_BUFF_LEN - 1)) {
		if(self->ctx->line_pos < self->ctx->line_len) {
			memmove(self->ctx->line_buffer + self->ctx->line_pos + 1,
				self->ctx->line_buffer + self->ctx->line_pos,
				self->ctx->line_len - self->ctx->line_pos+1);
		}

		self->ctx->line_buffer[self->ctx->line_pos] = c;
		self->ctx->line_len++;
		self->ctx->line_pos++;
		self->ctx->line_buffer[self->ctx->line_len] = '\0';

		error = eddy_put(self, c);

		if(self->ctx->line_pos < self->ctx->line_len) {
			if(!error) {
				eddy_print(self, VT100_SAVE_CURSOR_POS);
			}
		}

		if(self->ctx->line_pos < self->ctx->line_len) {
			if(!error) {
				error = eddy_print(self, self->ctx->line_buffer + self->ctx->line_pos);
			}

			if(!error) {
				error = eddy_print(self, VT100_RESTORE_CURSOR_POS);
			}
		}
	}

	return error;
}

/**
 * @brief Proceed back space on line buffer.
 * 
 * @param self Pointer on library context.
 * @return eddy_retv_t Error code: EDDY_RETV_OK if succes or EDDY_RETV_ERR if error.
 */
eddy_retv_t eddy_process_bs_key(eddy_p self)
{
	eddy_retv_t error;

	if(self->ctx->line_pos > 0) {
		error = eddy_print(self, VT100_BACKSPACE);

		if(self->ctx->line_pos < self->ctx->line_len) {
			error = eddy_print(self, VT100_SAVE_CURSOR_POS);

			if(!error) {
				error = eddy_print(self, self->ctx->line_buffer + self->ctx->line_pos);
			}

			if(!error) {
				error = eddy_print(self, " " VT100_RESTORE_CURSOR_POS);
			}

			memmove(self->ctx->line_buffer + self->ctx->line_pos - 1,
				self->ctx->line_buffer + self->ctx->line_pos,
				self->ctx->line_len - self->ctx->line_pos);
		} else {
			eddy_print(self, VT100_CLEAR_SCREEN_DOWN);
		}

		self->ctx->line_len--;
		self->ctx->line_pos--;
		self->ctx->line_buffer[self->ctx->line_len] = '\0';
	}

	return EDDY_RETV_OK;
}

/**
 * @brief Proceed delete on line buffer.
 * 
 * @param self Pointer on library context.
 * @return eddy_retv_t Error code: EDDY_RETV_OK if succes or EDDY_RETV_ERR if error.
 */
eddy_retv_t eddy_process_del_key(eddy_p self)
{
	eddy_retv_t error = EDDY_RETV_OK;

	if(self->ctx->line_pos < self->ctx->line_len) {
		error = eddy_print(self, VT100_SAVE_CURSOR_POS);

		if(!error) {
			error = eddy_print(self, self->ctx->line_buffer + self->ctx->line_pos + 1);
		}

		if(!error) {
			error = eddy_print(self, " " VT100_RESTORE_CURSOR_POS);
		}

		memmove(self->ctx->line_buffer + self->ctx->line_pos,
			self->ctx->line_buffer + self->ctx->line_pos + 1,
			self->ctx->line_len - self->ctx->line_pos+1);

		self->ctx->line_len--;
	}

	return error;
}

/**
 * @brief Proceed move cursor left on line buffer.
 * 
 * @param self Pointer on library context.
 * @return eddy_retv_t Error code: EDDY_RETV_OK if succes or EDDY_RETV_ERR if error.
 */
eddy_retv_t eddy_process_cursor_left(eddy_p self)
{
	eddy_retv_t error = EDDY_RETV_OK;

	if(self->ctx->line_pos > 0) {
		error = eddy_print(self, VT100_BACKSPACE);
		self->ctx->line_pos--;
	}

	return error;
}

/**
 * @brief Proceed move cursor right on line buffer.
 * 
 * @param self Pointer on library context
 * @return eddy_retv_t Error code: EDDY_RETV_OK if succes or EDDY_RETV_ERR if error.
 */
eddy_retv_t eddy_process_cursor_right(eddy_p self)
{
	eddy_retv_t error = EDDY_RETV_OK;

	if(self->ctx->line_pos < self->ctx->line_len) {
		error = eddy_print(self, VT100_MOVE_CURSOR_RIGHT);

		self->ctx->line_pos++;
	}

	return error;
}

/**
 * @brief Function proceed hint searching.
 * 
 * The function is called when [TAB] is pressed. Passes
 * the entered command through the set callback function.
 * 
 * @see eddy_s#set_check_hint_clbk
 * 
 * @param self Pointer on library context.
 * @param cmd_line Command line buffer to process.
 * @return eddy_retv_t Error code: EDDY_RETV_OK if succes or EDDY_RETV_ERR if error.
 */
eddy_retv_t eddy_process_check_hint(eddy_p self, char* cmd_line)
{
	eddy_retv_t error = EDDY_RETV_OK;

	if(self == EDDY_NULL || self->ctx->check_hint_clbk == EDDY_NULL) {
		return EDDY_RETV_ERR;
	}

	self->ctx->check_hint_clbk(cmd_line);

	self->ctx->line_pos = strlen(self->ctx->line_buffer);
	self->ctx->line_len = self->ctx->line_pos;

	if(!error) {
		error = eddy_print(self, self->ctx->prompt);
	}

	if(!error) {
		error = eddy_print(self, self->ctx->line_buffer);
	}

	return EDDY_RETV_OK;
}

/**
 * @brief Function precesses command entered in terminal.
 * 
 * The function is called when [ENTER] is pressed. Passes
 * the entered command through the set callback function.
 * 
 * @param self Pointer on library context.
 * @param cmd_line Command line buffer to process.
 * @return eddy_retv_t Error code: EDDY_RETV_OK if succes or EDDY_RETV_ERR if error.
 */
eddy_retv_t eddy_process_exec_cmd(eddy_p self, const char* cmd_line)
{
	eddy_retv_t error = EDDY_RETV_OK;

	if(self == EDDY_NULL || self->ctx->exec_cmd_clbk == EDDY_NULL) {
		return EDDY_RETV_ERR;
	}

	error = eddy_print(self, "\r\n");

	if(!error) {
		if(self->ctx->exec_cmd_clbk(cmd_line) != EDDY_RETV_OK) {
			error = eddy_print(self, "ERROR\r\n");
		}
	}

	if(!error) {
		error = eddy_print(self, self->ctx->prompt);
	}

	self->ctx->line_len = 0;
	self->ctx->line_pos = 0;
	self->ctx->line_buffer[self->ctx->line_len] = '\0';

	return error;
}

/**
 * @brief Function to print string in terminal.
 * 
 * @param self Pointer on library context.
 * @param buffer Pointer on buffer to print.
 * @return eddy_retv_t Error code: EDDY_RETV_OK if succes or EDDY_RETV_ERR if error.
 */
eddy_retv_t eddy_print(eddy_p self, const char* buffer)
{
	if(self == EDDY_NULL || self->ctx->cli_print_clbk == EDDY_NULL) {
		return EDDY_RETV_ERR;
	}

	self->ctx->cli_print_clbk(buffer);

	return EDDY_RETV_OK;
}

/**
 * @brief Function to print single character in terminal.
 * 
 * @param self Pointer on library context.
 * @param ch Character to print.
 * @return eddy_retv_t Error code: EDDY_RETV_OK if succes or EDDY_RETV_ERR if error.
 */
eddy_retv_t eddy_put(eddy_p self, char ch)
{
	char buffer[2];

	if(self == EDDY_NULL || self->ctx->cli_print_clbk == EDDY_NULL) {
		return EDDY_RETV_ERR;
	}

	buffer[0] = ch;
	buffer[1] = '\0';

	self->ctx->cli_print_clbk(buffer);

	return EDDY_RETV_OK;
}