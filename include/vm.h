#ifndef VM_H
#define VM_H

#include "utils.h"
#include <stddef.h>
#include <stdlib.h>

typedef uintptr_t word_t;
typedef intptr_t sword_t;

typedef struct {
	word_t* data;
	size_t len;
	size_t cap;
}Stack;

typedef enum : char {
    OP_DONE=0,
    OP_PUSH_CONST,
    OP_CALL,
    OP_RET,
    OP_CALL_DYN,
    OP_DROP,
    OP_DUP,
    
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    
    OP_PUSH_RS,
    OP_POP_RS,
    OP_PEEK_RS,

    OP_NEXT_TOKEN,
    OP_FIND_WORD,
    OP_COMPILE_CODE,
    OP_COMPILE_LOOP,
    OP_RUN_LOOP,
    OP_WORD_CALL_PTR,

    OP_DOT,
    OP_DOT_S,

    OP_FUNC_START,
    OP_NOW_FUNC_START,
    OP_FUNC_END,
    OP_FUNC_INLINE_END,
    OP_FUNC_OUTLINE_END,

    OP_LAST,
} code_t;

typedef struct {
	code_t* data;
	size_t len;
    //order is significant
	size_t cap;
}Comp;

typedef struct {
    code_t* data;
    size_t len;
}Code;

typedef struct {
    const char* start;
    const char* end;
}TextStream;

static inline TextStream next_token(TextStream* stream){
    const char* start = stream->start;
    
    while(start!=stream->end && *start==' ')
            start++;

    const char* end = start;
    while(end!=stream->end && *end!=' ')
            end++;

    stream->start=end;
    return (TextStream){start,end};
}

struct Lex;
typedef struct {
	word_t tos;
	Stack ds;
	Stack rs;
	Comp comp;
    TextStream input;
    //lex is not owned and wont be auto freed
    struct Lex* lex;
}VM;

void vm_init(VM* vm);
void vm_free(VM* vm);

#endif // VM_H

