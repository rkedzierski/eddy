/**
 * @file eddy_config.h
 * @author Rafał Kędzierski (rafal.kedzierski@gmail.com)
 * @brief Library configuration file
 * @version 0.1
 * @date 2023-04-22
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef __EDDY_CONFIG_H__
#define __EDDY_CONFIG_H__

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
 * @brief Declaration of size type
 * 
 */
typedef size_t clima_size_t;

/**
 * @brief Definition of memory allocation function
 * 
 * @param size in bytes
 * @return void* pointer to buffer
 */
weak static inline void* eddy_malloc(clima_size_t size) {
	return malloc(size);
}

#endif /* __EDDY_CONFIG_H__ */