#ifndef COMPILE_H
#define COMPILE_H

#include "vm.h"
#include "basic_ops.h"
#include "third_party/uthash.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>



typedef struct {
    Comp comp;
    bool is_inline:1;
    bool is_now:1;
} Word;


static inline void comp_push_word(Comp* comp,word_t c){
	ARR_ENSURE_CAP(*comp,comp->len+sizeof(c));
	comp->len+=sizeof(c);
	memcpy(&comp->data[comp->len-sizeof(c)],&c,sizeof(c));
}

static inline void comp_push_code(Comp* comp,code_t c){
	ARR_PUSH(*comp,c);
}

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

static inline void* xmalloc(size_t sz) {
    void* p = malloc(sz);
    if (!p) abort();
    return p;
}

static inline void* xrealloc(void* p, size_t sz) {
    p = realloc(p, sz);
    if (!p) abort();
    return p;
}

typedef struct LexEntry {
    char* name;
    Word* word;
    UT_hash_handle hh;
} LexEntry;

typedef struct {
    LexEntry* entries;
} Lex;

static inline void destroy_word(Word* word){
	free(word->comp.data);
	word->comp.data=NULL;
}

static inline void lex_init(Lex* lex) {
    lex->entries = NULL;
}

static inline void lex_free(Lex* lex) {
    LexEntry *entry, *tmp;
    HASH_ITER(hh, lex->entries, entry, tmp) {
        HASH_DELETE(hh, lex->entries, entry);
        free(entry->name);
        destroy_word(entry->word);
        free(entry->word);
        free(entry);
    }
}

static inline Word* lex_find(Lex* lex, const char* name, size_t name_len) {
    LexEntry* entry;
    HASH_FIND(hh, lex->entries, name, name_len, entry);
    return entry ? entry->word : NULL;
}

static inline void lex_define(Lex* lex, const char* name, size_t name_len, Word* word) {
    LexEntry* entry;
    entry = (LexEntry*)xmalloc(sizeof(LexEntry));
    entry->name = (char*)xmalloc(name_len + 1);
    memcpy(entry->name, name, name_len);
    entry->name[name_len] = '\0';
    entry->word = word;
    HASH_ADD_KEYPTR(hh, lex->entries, entry->name, name_len, entry);
}

#endif // COMPILE_H

