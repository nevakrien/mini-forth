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

static inline void compile_word(VM* vm, const Word* w){
    if(w->is_now)
        run_vm(vm, w->code.data);
    else
        compile_later(&vm->comp, w);
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
    struct { code_t op; char* name; } known[] = {
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
        { OP_WORD_CALL_PTR, "run-word" },
        { OP_DOT, "." },
        { OP_DOT_S, ".s" },
    };
    size_t num_ops = sizeof(known) / sizeof(known[0]);
    for(size_t i=0;i<num_ops;i++){
        Word* word = lex_define(lex, known[i].name, strlen(known[i].name));
        word->is_inline = true;
        word->code.len = 2;
        word->code.data = xmalloc(sizeof(code_t) * 2);
        word->code.data[0] = known[i].op;
        word->code.data[1] = OP_RET;
    }
}

static inline int parse_number(word_t* ans,const char* name, size_t name_len){
    bool saw_minus=false;
    *ans = 0;
    if(name_len==0) return -1;
    if(name[0]=='-'){
        saw_minus=true;
        name+=1;
        name_len-=1;
    }

    for(size_t i=0;i<name_len;i++){
        unsigned char c = name[i];
        c-='0';
        if(c>9) return -1;
        *ans=*ans*10+c;
    }
    if(saw_minus){
        *ans*=-1;
    }
    return 0;
}

static inline int compile_token(VM* vm,const char* name, size_t name_len){
    Word* word =lex_find(vm->lex,name,name_len);
    if(word) {
        compile_word(vm,word);
        return 0;
    }

    word_t num = 0;
    if(parse_number(&num,name,name_len)) return 1;

    comp_push_code(&vm->comp,OP_PUSH_CONST);
    comp_push_word(&vm->comp,num);
    return 0;
}

static inline int compile_text(VM* vm,const char* text, const char* text_end){
    const char* tok_start = text;
    
    while(tok_start!=text_end && *tok_start==' ')
            tok_start++;

    const char* tok_end = tok_start;
    while(tok_end!=text_end && *tok_end!=' ')
            tok_end++;

    size_t tok_len = tok_end-tok_start;
    if(tok_len == 0) return 0;

    if(compile_token(vm,tok_start,tok_len)){
        //TODO print the error with the unrecognized name
        return 1;
    }

    return compile_text(vm,tok_end,text_end);
}

static inline int run_text(VM* vm,const char* text, const char* text_end){
    if(compile_text(vm,text,text_end))  // compile_text returns 0 on success
        return 1;

    ARR_PUSH(vm->comp,OP_DONE);
    run_vm(vm,vm->comp.data);
    vm->comp.len=0;
    return 0;
}

static inline int run_lines(VM* vm,const char* text, size_t text_len){
    const char* cur = text;
    const char* end = text+text_len;
    while(cur<end){
        const char* line_end = cur;
        while(line_end<end && *line_end!='\n') line_end++;
        if(run_text(vm,cur,line_end)) return 1;
        cur=line_end;
        if(cur<end) cur++;
    } 
}

#endif // COMPILE_H

