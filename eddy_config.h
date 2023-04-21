#ifndef __EDDY_CONFIG_H__
#define __EDDY_CONFIG_H__

#include <stdlib.h>
#include <stddef.h>

#define EDDY_MAX_LINE_BUFF_LEN	256

typedef size_t clima_size_t;

static inline void* eddy_malloc(clima_size_t size) {
	return malloc(size);
}

#endif /* __EDDY_CONFIG_H__ */