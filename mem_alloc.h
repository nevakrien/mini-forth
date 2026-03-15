#ifndef MEM_ALLOC_H
#define MEM_ALLOC_H

#include "utils.h"

static inline void ensure_mem(void** ptr,size_t size){
	*ptr = realloc(*ptr,size);
	assert(ptr && "went oom");
	return ptr;
}

#endif // MEM_ALLOC_H

