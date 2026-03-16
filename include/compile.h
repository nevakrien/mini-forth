#ifndef COMPILE_H
#define COMPILE_H

#include "vm.h"
#include "basic_ops.h"
#include "third_party/uthash.h"

typedef struct {
    Code code;
    bool is_inline:1;
    bool is_now:1;
} Word;


typedef struct LexEntry {
    Word word;
    UT_hash_handle hh;
} LexEntry;

typedef struct Lex{
    LexEntry* entries;
} Lex;


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
        comp_push_word(comp, (word_t)w->code.data);
        return;
    }

    for(size_t i = 0; i < w->code.len-1; i++){
        comp_push_code(comp, w->code.data[i]);
    }
}

static inline void destroy_word(Word* word){
	free(word->code.data);
	word->code.data=NULL;
}

static inline void lex_free(Lex* lex) {
    LexEntry *entry, *tmp;
    HASH_ITER(hh, lex->entries, entry, tmp) {
        HASH_DELETE(hh, lex->entries, entry);
        free((void*)entry->hh.key);
        destroy_word(&entry->word);
        free(entry);
    }
}

static inline Word* lex_find(Lex* lex, const char* name, size_t name_len) {
    LexEntry* entry;
    HASH_FIND(hh, lex->entries, name, name_len, entry);
    return entry ? &entry->word : NULL;
}

static inline Word* lex_define(Lex* lex, const char* name, size_t name_len) {
    LexEntry* entry;
    entry = (LexEntry*)xmalloc(sizeof(LexEntry));
    char* owned = (char*)xmalloc(name_len + 1);
    memcpy(owned, name, name_len);
    owned[name_len] = '\0';
    entry->word = (Word){0};
    HASH_ADD_KEYPTR(hh, lex->entries, owned, name_len, entry);
    return &entry->word;
}

static inline void lex_init(Lex* lex) {
    lex->entries=NULL;

    struct { code_t op; char* name; } simple[] = {
        { OP_DROP, "drop" },
        { OP_DUP, "dup" },
        { OP_ADD, "+" },
        { OP_SUB, "-" },
        { OP_MUL, "*" },
        { OP_DIV, "/" },
        { OP_MOD, "%" },
        { OP_PUSH_RS, ">r" },
        { OP_POP_RS, "r>" },
        { OP_PEEK_RS, "r@" },
        { OP_DONE, "bye" },
        { OP_COMPILE_CODE, "compile," },
        { OP_COMPILE_LOOP, "compile-loop" },
        { OP_WORD_CALL_PTR, "run-word" },
        { OP_FIND_WORD, "find-word" },
        { OP_NEXT_TOKEN, "next-token" },
        { OP_DOT, "." },
        { OP_DOT_S, ".s" },
    };
    size_t num_ops = sizeof(simple) / sizeof(simple[0]);
    for(size_t i=0;i<num_ops;i++){
        Word* word = lex_define(lex, simple[i].name, strlen(simple[i].name));
        word->is_inline = true;
        word->code.len = 2;
        word->code.data = xmalloc(sizeof(code_t) * 2);
        word->code.data[0] = simple[i].op;
        word->code.data[1] = OP_RET;
    }

    // struct { code_t op[10]; char* name; } now[] = {
    //     [OP_]
    // };
}


static inline void run_text(VM* vm){
    code_t code[]={OP_RUN_LOOP,OP_DONE};
    run_vm(vm,code);
    return;
}



#endif // COMPILE_H

