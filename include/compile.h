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

typedef enum : word_t{
    COMP_TAG_FUNC,
    COMP_TAG_END,//generic end of scope (then/again/repeat)

    COMP_TAG_IF,
    COMP_TAG_ELSE,

    // COMP_TAG_BEGIN,
    // COMP_TAG_UNTIL,
    // COMP_TAG_WHILE,
    // COMP_TAG_BREAK,
    // COMP_TAG_REPEAT,
} comp_tag_t;

static inline void comp_push_word(Comp* comp,word_t c){
	ARR_ENSURE_CAP(*comp,comp->len+sizeof(c));
	comp->len+=sizeof(c);
	memcpy(&comp->data[comp->len-sizeof(c)],&c,sizeof(c));
}

static inline void comp_push_offset(Comp* comp,boffset_t c){
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

static inline LexEntry* make_lex_entry(const char* name, size_t name_len) {
    LexEntry* entry;
    entry = (LexEntry*)xmalloc(sizeof(LexEntry));
    char* owned = (char*)xmalloc(name_len + 1);
    memcpy(owned, name, name_len);
    owned[name_len] = '\0';
    entry->hh.key=owned;
    entry->hh.keylen=name_len;
    entry->word = (Word){0};
    return entry;
}

static inline void lex_insert_entry(Lex* lex,LexEntry* entry){
    HASH_ADD_KEYPTR(hh, lex->entries, entry->hh.key, entry->hh.keylen, entry);
}

static inline Word* lex_define(Lex* lex, const char* name, size_t name_len) {
    LexEntry* entry = make_lex_entry(name,name_len); 
    lex_insert_entry(lex,entry);   
    return &entry->word;
}

static inline void lex_init(Lex* lex) {
    lex->entries=NULL;

    struct { code_t op; char* name; } simple[] = {
        { OP_DROP, "drop" },
        { OP_DUP, "dup" },
        { OP_SWAP, "swap" },
        { OP_OVER, "over" },
        { OP_NIP, "nip" },
        { OP_TUCK, "tuck" },
        { OP_ROT, "rot" },
        { OP_NROT, "-rot" },
        { OP_2DUP, "2dup" },
        { OP_2DROP, "2drop" },
        { OP_2SWAP, "2swap" },
        { OP_2OVER, "2over" },
        { OP_PICK, "pick" },
        { OP_ROLL, "roll" },
        { OP_ADD, "+" },
        { OP_SUB, "-" },
        { OP_MUL, "*" },
        { OP_DIV, "/" },
        { OP_MOD, "%" },
        { OP_EQ, "=" },
        { OP_NE, "~=" },
        { OP_LT, "<" },
        { OP_GT, ">" },
        { OP_LE, "<=" },
        { OP_GE, ">=" },
        { OP_ZEQ, "0=" },
        { OP_ZNE, "0~=" },
        { OP_BIT_AND, "&" },
        { OP_BIT_OR, "|" },
        { OP_BIT_XOR, "^" },
        { OP_BIT_NOT, "~" },
        { OP_SHL, "<<" },
        { OP_SHR, ">>" },
        { OP_PUSH_RS, ">r" },
        { OP_POP_RS, "r>" },
        { OP_PEEK_RS, "r@" },
        { OP_RET, "ret" },
        { OP_DONE, "bye" },
        { OP_CRASH, "crash" },
        { OP_COMPILE_CODE, "compile," },
        { OP_COMPILE_LOOP, "compile-loop" },
        { OP_WORD_CALL_PTR, "run-word" },
        { OP_FIND_WORD, "find-word" },
        { OP_NEXT_TOKEN, "next-token" },
        { OP_DOT, "." },
        { OP_DOT_S, ".s" },
        { OP_COMP_IDX, "comp-idx" },
        { OP_COMPILE_JUMP, "compile-jump" },
        { OP_COMPILE_BRANCH, "compile-branch" },
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

    struct { code_t op; char* name; } simple_now[] = {
        { OP_COMPILE_CONST_PRINT, ".\"" },
        { OP_FUNC_START, ":" },
        { OP_NOW_FUNC_START, "now:" },
        { OP_FUNC_END, ";" },
        { OP_FUNC_INLINE_END, ";inline" },
        { OP_FUNC_OUTLINE_END, ";outline" },
        { OP_COMPILE_IF, "if" },
        { OP_COMPILE_ELSE, "else" },
        { OP_COMPILE_END, "end" },
    };
    num_ops = sizeof(simple_now) / sizeof(simple_now[0]);
    
    for(size_t i=0;i<num_ops;i++){
        Word* word = lex_define(lex, simple_now[i].name, strlen(simple_now[i].name));
        word->is_inline = true;
        word->is_now = true;
        word->code.len = 2;
        word->code.data = xmalloc(sizeof(code_t) * 2);
        word->code.data[0] = simple_now[i].op;
        word->code.data[1] = OP_RET;
    }

}


static inline StopReason run_text(VM* vm){
    static const code_t code[]={OP_RUN_LOOP,OP_DONE};
    return run_vm(vm,code);
}

static inline StopReason run_compile_text(VM* vm){
    static const code_t code[]={OP_COMPILE_LOOP,OP_DONE};
    return run_vm(vm,code);
}

// static inline void run_repl_text(VM* vm){
//     //construct [loop print "ok" done]
//     code_t code[2+sizeof(word_t)+4]={OP_RUN_LOOP,OP_CONST_PRINT};
//     word_t len = 3;
//     code_t code_end[4] = {' ','o','k',OP_DONE};

//     memcpy(code+2,&len,sizeof(word_t));
//     memcpy(code+2+sizeof(word_t),code_end,sizeof(code_end));

//     run_vm(vm,code);
//     return;
// }


#endif // COMPILE_H
