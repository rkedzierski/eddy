#include "eddy.h"
#include "eddy_config.h"

#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define EDDY_NULL 0

//Escape character code
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

#define EDDY_MAX_ESC_SEQ_LEN		7
#define EDDY_MAX_PROMPT_LEN			8

typedef struct eddy_keys_codes_s {
	char bs_key;
	char del_key;
} eddy_keys_codes_t;

typedef struct eddy_ctx_s {
	char line_buffer[EDDY_MAX_LINE_BUFF_LEN];
	unsigned int line_len;
	unsigned int line_pos;
	char esc_seq[EDDY_MAX_ESC_SEQ_LEN+1];
	unsigned int esc_seq_len;
	char prompt[EDDY_MAX_PROMPT_LEN];
	eddy_keys_codes_t keys_codes;

	eddy_cli_print_clbk cli_print_clbk;
	eddy_log_print_clbk log_print_clbk;
	eddy_check_hint_clbk check_hint_clbk;
	eddy_exec_cmd_clbk exec_cmd_clbk;
} eddy_ctx_t;

eddy_retv_t eddy_put_char_impl(eddy_p self, char c);
eddy_retv_t eddy_set_cli_print_impl(eddy_p self, eddy_cli_print_clbk cli_print_clbk);
eddy_retv_t eddy_set_log_print_impl(eddy_p self, eddy_log_print_clbk log_print_clbk);
eddy_retv_t eddy_set_check_hint_impl(eddy_p self, eddy_check_hint_clbk check_hint_clbk);
eddy_retv_t eddy_set_exec_cmd_impl(eddy_p self, eddy_exec_cmd_clbk exec_cmd_clbk);

eddy_retv_t eddy_proces_insert_char(eddy_p self, char c);
eddy_retv_t eddy_process_cursor_left(eddy_p self);
eddy_retv_t eddy_process_cursor_right(eddy_p self);
eddy_retv_t eddy_process_check_hint(eddy_p self, char* cmd_line);
eddy_retv_t eddy_process_exec_cmd(eddy_p self, const char* cmd_line);
eddy_retv_t eddy_process_bs_key(eddy_p self);
eddy_retv_t eddy_process_del_key(eddy_p self);
eddy_retv_t eddy_write(eddy_p self, const char* buffer);
eddy_retv_t eddy_put(eddy_p self, char chr);

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

	self->ctx->keys_codes.bs_key = VT100_DEL_CODE; //VT100_BS_CODE;
	self->ctx->keys_codes.del_key = VT100_BS_CODE;

	self->ctx->line_len = 0;
	self->ctx->line_pos = 0;
	self->ctx->esc_seq_len = 0;

	sprintf(self->ctx->prompt, ">");
	eddy_write(self, self->ctx->prompt);

	return EDDY_RETV_OK;
}

eddy_retv_t eddy_set_cli_print_impl(eddy_p self, eddy_cli_print_clbk cli_print_clbk)
{
	if(self == EDDY_NULL || cli_print_clbk == EDDY_NULL) {
		return EDDY_RETV_ERR;
	}

	self->ctx->cli_print_clbk = cli_print_clbk;

	return EDDY_RETV_OK;
}

eddy_retv_t eddy_set_log_print_impl(eddy_p self, eddy_log_print_clbk log_print_clbk)
{
	if(self == EDDY_NULL || log_print_clbk == EDDY_NULL) {
		return EDDY_RETV_ERR;
	}

	self->ctx->log_print_clbk = log_print_clbk;

	return EDDY_RETV_OK;
}

eddy_retv_t eddy_set_check_hint_impl(eddy_p self, eddy_check_hint_clbk check_hint_clbk)
{
	if(self == EDDY_NULL || check_hint_clbk == EDDY_NULL) {
		return EDDY_RETV_ERR;
	}

	self->ctx->check_hint_clbk = check_hint_clbk;

	return EDDY_RETV_OK;
}

eddy_retv_t eddy_set_exec_cmd_impl(eddy_p self, eddy_exec_cmd_clbk exec_cmd_clbk)
{
	if(self == EDDY_NULL || exec_cmd_clbk == EDDY_NULL) {
		return EDDY_RETV_ERR;
	}

	self->ctx->exec_cmd_clbk = exec_cmd_clbk;

	return EDDY_RETV_OK;
}

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
				eddy_write(self, self->ctx->esc_seq);
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

eddy_retv_t eddy_proces_insert_char(eddy_p self, char c)
{
	eddy_retv_t error = EDDY_RETV_OK;
	//unsigned int cursorPos;

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

		//if((cursorPos % session->termWidth) == 0)
		//{
		//   osStrcat(buffer, "\r\n");
		//}

		if(self->ctx->line_pos < self->ctx->line_len) {
			if(!error) {
				eddy_write(self, VT100_SAVE_CURSOR_POS);
			}
		}

		if(self->ctx->line_pos < self->ctx->line_len) {
			if(!error) {
				error = eddy_write(self, self->ctx->line_buffer + self->ctx->line_pos);
			}

			if(!error) {
				error = eddy_write(self, VT100_RESTORE_CURSOR_POS);
			}
		}
	}

	return error;
}

eddy_retv_t eddy_process_bs_key(eddy_p self)
{
	eddy_retv_t error;

	if(self->ctx->line_pos > 0) {
		error = eddy_write(self, VT100_BACKSPACE);

		if(self->ctx->line_pos < self->ctx->line_len) {
			error = eddy_write(self, VT100_SAVE_CURSOR_POS);

			if(!error) {
				error = eddy_write(self, self->ctx->line_buffer + self->ctx->line_pos);
			}

			if(!error) {
				error = eddy_write(self, " " VT100_RESTORE_CURSOR_POS);
			}

			memmove(self->ctx->line_buffer + self->ctx->line_pos - 1,
				self->ctx->line_buffer + self->ctx->line_pos,
				self->ctx->line_len - self->ctx->line_pos);
		} else {
			eddy_write(self, VT100_CLEAR_SCREEN_DOWN);
		}

		self->ctx->line_len--;
		self->ctx->line_pos--;
		self->ctx->line_buffer[self->ctx->line_len] = '\0';
	}

	return EDDY_RETV_OK;
}

eddy_retv_t eddy_process_del_key(eddy_p self)
{
	eddy_retv_t error = EDDY_RETV_OK;

	if(self->ctx->line_pos < self->ctx->line_len) {
		error = eddy_write(self, VT100_SAVE_CURSOR_POS);

		if(!error) {
			error = eddy_write(self, self->ctx->line_buffer + self->ctx->line_pos + 1);
		}

		if(!error) {
			error = eddy_write(self, " " VT100_RESTORE_CURSOR_POS);
		}

		memmove(self->ctx->line_buffer + self->ctx->line_pos,
			self->ctx->line_buffer + self->ctx->line_pos + 1,
			self->ctx->line_len - self->ctx->line_pos+1);

		self->ctx->line_len--;
	}

	return error;
}

eddy_retv_t eddy_process_cursor_left(eddy_p self)
{
	eddy_retv_t error = EDDY_RETV_OK;
	//unsigned int cursorPos;
	//char buffer[16];

	if(self->ctx->line_pos > 0) {
		//Determine the current position of the cursor
		//cursorPos = session->promptLen + session->bufferPos;

		//Moving left at the edge of the screen wraps to the previous line
		//if((cursorPos % session->termWidth) == 0)
		//{
		//   osSprintf(buffer, VT100_MOVE_CURSOR_UP VT100_MOVE_CURSOR_RIGHT_N,
		//      (uint_t) (session->termWidth - 1));
		//	error = eddy_write(self, buffer);
		//}
		//else
		//{
			error = eddy_write(self, VT100_BACKSPACE);
		//}

		self->ctx->line_pos--;
	}

	return error;
}

eddy_retv_t eddy_process_cursor_right(eddy_p self)
{
	eddy_retv_t error = EDDY_RETV_OK;
	//unsigned int cursorPos;

	if(self->ctx->line_pos < self->ctx->line_len) {
		//Determine the current position of the cursor
		//cursorPos = session->promptLen + session->bufferPos;

		//Moving right at the edge of the screen wraps to the next line
		//if((cursorPos % session->termWidth) == (session->termWidth - 1))
		//{
		//	error = eddy_write(self, "\r\n");
		//}
		//else
		//{
			error = eddy_write(self, VT100_MOVE_CURSOR_RIGHT);
		//}

		self->ctx->line_pos++;
	}

	return error;
}

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
		error = eddy_write(self, self->ctx->prompt);
	}

	if(!error) {
		error = eddy_write(self, self->ctx->line_buffer);
	}

	return EDDY_RETV_OK;
}

eddy_retv_t eddy_process_exec_cmd(eddy_p self, const char* cmd_line)
{
	eddy_retv_t error = EDDY_RETV_OK;

	if(self == EDDY_NULL || self->ctx->exec_cmd_clbk == EDDY_NULL) {
		return EDDY_RETV_ERR;
	}

	error = eddy_write(self, "\r\n");

	if(!error) {
		if(self->ctx->exec_cmd_clbk(cmd_line) != EDDY_RETV_OK) {
			error = eddy_write(self, "ERROR\r\n");
		}
	}

	if(!error) {
		error = eddy_write(self, self->ctx->prompt);
	}

	self->ctx->line_len = 0;
	self->ctx->line_pos = 0;
	self->ctx->line_buffer[self->ctx->line_len] = '\0';

	return error;
}

eddy_retv_t eddy_write(eddy_p self, const char* buffer)
{
	if(self == EDDY_NULL || self->ctx->cli_print_clbk == EDDY_NULL) {
		return EDDY_RETV_ERR;
	}

	self->ctx->cli_print_clbk(buffer);

	return EDDY_RETV_OK;
}

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