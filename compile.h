#ifndef COMPILE_H
#define COMPILE_H

#include "vm.h"
#include "basic_ops.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

static inline void comp_push_word(Comp* comp,word_t c){
	ARR_ENSURE_CAP(*comp,comp->len+sizeof(c));
	comp->len+=sizeof(c);
	memcpy(&comp->data[comp->len-sizeof(c)],&c,sizeof(c));
}

static inline void comp_push_code(Comp* comp,code_t c){
	ARR_PUSH(*comp,c);
}

typedef struct {
    Comp comp;
    bool is_inline:1;
    bool is_now:1;
} Word;

static inline void compile_later(Comp* comp, const Word* w){
    if(!w->is_inline){
        comp_push_code(comp, OP_CALL);
        comp_push_word(comp, (word_t)w->comp.data);
        return;
    }

    for(size_t i = 0; i < w->comp.len; i++){
        comp_push_code(comp, w->comp.data[i]);
    }
}

static inline void compile_word(VM* vm, const Word* w){
    if(w->is_now)
        run_vm(vm, w->comp.data);
    else
        compile_later(&vm->comp, w);
}


#endif // COMPILE_H

