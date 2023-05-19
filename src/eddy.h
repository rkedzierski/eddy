/**
 * @file eddy.h
 * @author Rafał Kędzierski (rafal.kedzierski@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2023-04-21
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef __EDDY_H__
#define __EDDY_H__

#include <stdlib.h>
#include <stddef.h>

/**
 * @brief Size of line buffer
 * 
 */
#ifndef EDDY_MAX_LINE_BUFF_LEN
#define EDDY_MAX_LINE_BUFF_LEN	256
#endif

/**
 * @brief Library return type
 */
typedef enum eddy_retv_e {
    EDDY_RETV_OK,   /**< returned if everyfing is OK */
    EDDY_RETV_ERR,  /**< returned if error */
} eddy_retv_t;

//typedef int eddy_size_t;

#ifndef eddy_size_t
/**
 * @brief Default declaration of eddy size type[replaceable]
 * 
 */
typedef size_t eddy_size_t;
#endif


/**
 * @brief Forward declarations
 * @{
 */
struct eddy_ctx_s;
struct eddy_s;
/**
 * @}
 */

/**
 * @brief Definition of eddy_ctx_s struct type.
 * @see eddy_ctx_s
 */
typedef struct eddy_ctx_s eddy_ctx_t;

/**
 * @brief Definition of pointer on eddy_ctx_s struct type.
 * @see eddy_ctx_s
 */
typedef struct eddy_ctx_s* eddy_ctx_p;

/**
 * @brief Definition of eddy_s struct type.
 * @see eddy_s
 */
typedef struct eddy_s eddy_t;

/**
 * @brief Definition of pointer on eddy_s struct type.
 * @see eddy_s
 */
typedef struct eddy_s* eddy_p;

/**
 * @brief Pointer on print to terminal callback functrion.
 * @param string Pointer on buffer to print
 */
typedef void (*eddy_cli_print_clbk)(const char* string);

/**
 * @brief Pointer on log's print callback function.
 * @param string Pointer on buffer to print.
 */
typedef void (*eddy_log_print_clbk)(const char* string);

/**
 * @brief Pointer on check and print callback function.
 * @param cmd_line Pointer on line buffer to check.
 */
typedef void (*eddy_check_hint_clbk)(char* cmd_line);

/**
 * @{ \name Pointers on API functions.
 */
typedef eddy_retv_t (*eddy_exec_cmd_clbk)(const char* cmd_line);
typedef eddy_retv_t (*eddy_set_cli_print_clbk)(eddy_p self, eddy_cli_print_clbk cli_print_clbk);
typedef eddy_retv_t (*eddy_set_log_print_clbk)(eddy_p self, eddy_log_print_clbk log_print_clbk);
typedef eddy_retv_t (*eddy_set_check_hint_clbk)(eddy_p self, eddy_check_hint_clbk check_hint_clbk);
typedef eddy_retv_t (*eddy_set_exec_cmd_clbk)(eddy_p self, eddy_exec_cmd_clbk exec_cmd_clbk);
typedef eddy_retv_t (*eddy_put_char)(eddy_p self, char c);
typedef eddy_retv_t (*eddy_show_prompt)(eddy_p self);
/**
 * @}
 */

/**
 * @brief Library initialization function
 * 
 * Initialize library context. It is posible to init many contextes.
 * 
 * @param self Pointer on library context.
 * @return eddy_retv_t Error code: EDDY_RETV_OK if succes or EDDY_RETV_ERR if error.
 */
eddy_retv_t init_eddy(eddy_p self);

/**
 * @brief Eddy malloc function implementation. [replaceable]
 * 
 * Fuction have internal default implementation with statdard malloc call.
 * Function implementation can be replaced because default function is defined with WEAK.
 * 
 * @param size Size of memory to allocation.
 * @return void* Pointer to allocated memory.
 */
void* eddy_malloc(eddy_size_t size);

/**
 * @brief Eddy free function implementation. [replaceable]
 * 
 * Fuction have internal default implementation with statdard free call.
 * Function implementation can be replaced because default function is defined with WEAK.
 * 
 * @param ptr 
 */
void eddy_free(void *ptr);

/**
 * @brief Library context with API
 * 
 * Example of use:
 * 
 *     eddy_t eddy;
 * 
 *     init_eddy(&eddy);
 *     eddy.set_cli_print_clbk(&eddy, print_cli);
 *     eddy.set_check_hint_clbk(&eddy, check_hint);
 *     eddy.set_exec_cmd_clbk(&eddy, exec_command);
 * 
 *     while(some end condition) {
 *         cin = getchar();
 *         eddy.put_char(&eddy, cin);
 *     }
 */
struct eddy_s {
    /**
     * @{ \name Library context.
    */
    eddy_ctx_p ctx; /**< Internal private context.*/
    /**
     * @}
     */

    /**
     * @{ \name Library API 
    */
    eddy_put_char put_char; /**< Function to passes single character or key code from terminal. @see eddy_put_char_impl */
    eddy_set_cli_print_clbk set_cli_print_clbk; /**< To set terminal printing callback function @see eddy_set_cli_print_impl */
    eddy_set_log_print_clbk set_log_print_clbk; /**< To set logs printing callback function @see eddy_set_log_print_impl */
    eddy_set_check_hint_clbk set_check_hint_clbk; /**< To set check and print hint for command callback @see eddy_set_check_hint_impl */
    eddy_set_exec_cmd_clbk set_exec_cmd_clbk; /**<To set execute command callback function @see eddy_set_exec_cmd_impl */
    eddy_show_prompt show_prompt; /**< To show prompt first time. @see eddy_show_prompt_impl */
    /**
     * @}
     */
};

#endif /* __EDDY_H__ */