#ifndef __EDDY_H__
#define __EDDY_H__

typedef enum eddy_retv_e {
    EDDY_RETV_OK,
    EDDY_RETV_ERR,
} eddy_retv_t;

struct eddy_ctx_s;
typedef struct eddy_ctx_s eddy_ctx_t;
typedef struct eddy_ctx_s* eddy_ctx_p;

struct eddy_s;
typedef struct eddy_s eddy_t;
typedef struct eddy_s* eddy_p;

typedef void (*eddy_cli_print_clbk)(const char* string);

typedef void (*eddy_log_print_clbk)(const char* string);

typedef void (*eddy_check_hint_clbk)(char* cmd_line);

typedef eddy_retv_t (*eddy_exec_cmd_clbk)(const char* cmd_line);

typedef eddy_retv_t (*eddy_set_cli_print_clbk)(eddy_p self, eddy_cli_print_clbk cli_print_clbk);

typedef eddy_retv_t (*eddy_set_log_print_clbk)(eddy_p self, eddy_log_print_clbk log_print_clbk);

typedef eddy_retv_t (*eddy_set_check_hint_clbk)(eddy_p self, eddy_check_hint_clbk check_hint_clbk);

typedef eddy_retv_t (*eddy_set_exec_cmd_clbk)(eddy_p self, eddy_exec_cmd_clbk exec_cmd_clbk);

typedef eddy_retv_t (*eddy_put_char)(eddy_p self, char c);

eddy_retv_t init_eddy(eddy_p self);

struct eddy_s {
    eddy_ctx_p ctx;

    eddy_put_char put_char;
    
    eddy_set_cli_print_clbk set_cli_print_clbk;
    eddy_set_log_print_clbk set_log_print_clbk;
    eddy_set_check_hint_clbk set_check_hint_clbk;
    eddy_set_exec_cmd_clbk set_exec_cmd_clbk;

};

#endif /* __EDDY_H__ */