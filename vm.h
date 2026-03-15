#ifndef VM_H
#define VM_H
#include "utils.h"
#include <stddef.h>

typedef uintptr_t word_t;

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
} code_t;

typedef struct {
	code_t* data;
	size_t len;
	size_t cap;
}Comp;

typedef struct {
	word_t tos;
	Stack ds;
	Stack rs;
	Comp comp;
}VM;

#endif // VM_H

