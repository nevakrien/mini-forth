#ifndef COMPILE_H
#define COMPILE_H

#include "vm.h"
#include <stdlib.h>
#include <string.h>

static inline void comp_push_word(Comp* comp,word_t c){
	ARR_ENSURE_CAP(*comp);
	comp->len+=sizeof(c);
	memcpy(&comp->data[comp->len-sizeof(c)],&c,sizeof(c));
}

static inline void comp_push_code(Comp* comp,code_t c){
	ARR_PUSH(*comp,c);
}

#endif // COMPILE_H

