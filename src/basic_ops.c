#include "basic_ops.h"
#include "compile.h"
#include "utils.h"
#include <stdio.h>

#define PUSH(x) (ARR_PUSH(vm->ds, vm->tos), vm->tos = x)
#define DROP() (vm->tos = ARR_POP(vm->ds))

static inline Code take_comp(VM *vm) {
  Code ans = {0};
  ans.data = vm->comp.data;
  ans.len = vm->comp.len;
  vm->comp = (Comp){0};

  return ans;
}

static inline int parse_number(word_t *ans, const char *name, size_t name_len) {
  bool saw_minus = false;
  *ans = 0;
  if (name_len == 0)
    return -1;
  if (name[0] == '-') {
    saw_minus = true;
    name += 1;
    name_len -= 1;
  }

  for (size_t i = 0; i < name_len; i++) {
    unsigned char c = name[i];
    c -= '0';
    if (c > 9)
      return -1;
    *ans = *ans * 10 + c;
  }
  if (saw_minus) {
    *ans *= -1;
  }
  return 0;
}

void run_vm(VM *vm, const code_t *code) {
  // puts("run_vm start");

  static const void *const dispatch[] = {
      [OP_DONE] = &&op_done,
      [OP_PUSH_CONST] = &&op_push_const,
      [OP_CALL] = &&op_call,
      [OP_CALL_DYN] = &&op_call_dyn,
      [OP_RET] = &&op_ret,
      [OP_DROP] = &&op_drop,
      [OP_DUP] = &&op_dup,
      [OP_ADD] = &&op_add,
      [OP_SUB] = &&op_sub,
      [OP_MUL] = &&op_mul,
      [OP_DIV] = &&op_div,
      [OP_MOD] = &&op_mod,
      [OP_PUSH_RS] = &&op_push_rs,
      [OP_POP_RS] = &&op_pop_rs,
      [OP_PEEK_RS] = &&op_peek_rs,
      [OP_COMPILE_CODE] = &&op_compile_code,
      [OP_WORD_CALL_PTR] = &&op_word_call_ptr,
      [OP_NEXT_TOKEN] = &&op_next_token,
      [OP_FIND_WORD] = &&op_find_word,
      [OP_COMPILE_LOOP] = &&op_compile_loop,
      [OP_RUN_LOOP] = &&op_run_loop,
      [OP_DOT] = &&op_dot,
      [OP_DOT_S] = &&op_dot_s,
      [OP_FUNC_START] = &&op_func_start,
      [OP_NOW_FUNC_START] = &&op_now_func_start,
      [OP_FUNC_END] = &&op_func_end,
      [OP_FUNC_INLINE_END] = &&op_func_inline_end,
      [OP_FUNC_OUTLINE_END] = &&op_func_outline_end,
      [OP_PICK] = &&op_pick,
      [OP_ROLL] = &&op_roll,

  };

  // static const code_t code_run_loop[]  = {OP_RUN_LOOP,OP_RET};
  static const code_t code_compile_loop[] = {OP_COMPILE_LOOP, OP_RET};

#define DISPATCH() goto *dispatch[*++code]
#define DISPATCH_STAY() goto *dispatch[*code]
#define DISPATCH_CALL(tgt)                                                     \
  do {                                                                         \
    ARR_PUSH(vm->rs, (word_t)(code + 1));                                      \
    code = (tgt);                                                              \
    DISPATCH_STAY();                                                           \
  } while (0)

#define DISPATCH_LOOPING_CALL(tgt)                                             \
  do {                                                                         \
    ARR_PUSH(vm->rs, (word_t)(code));                                          \
    code = (tgt);                                                              \
    DISPATCH_STAY();                                                           \
  } while (0)

  DISPATCH_STAY();

op_done:
  return;

op_push_const: {
  word_t c = 0;
  memcpy(&c, ++code, sizeof(word_t));
  code += sizeof(word_t);
  PUSH(c);
  DISPATCH_STAY();
}

op_ret:
  code = (code_t *)ARR_POP(vm->rs);
  DISPATCH_STAY();

op_pop_rs:
  PUSH(ARR_POP(vm->rs));
  DISPATCH();

op_peek_rs:
  PUSH(ARR_PEEK(vm->rs));
  DISPATCH();

op_push_rs:
  ARR_PUSH(vm->rs, vm->tos);
  DROP();
  DISPATCH();

op_drop:
  DROP();
  DISPATCH();

op_dup:
  PUSH(vm->tos);
  DISPATCH();

op_pick: {
    word_t depth = vm->tos;
    word_t idx = vm->ds.len - depth-1;
    vm->tos = ARR_AT(vm->ds, idx);
    DISPATCH();
}

op_roll: {
    word_t depth = vm->tos;
    word_t idx = vm->ds.len - depth-1;
    
    vm->tos = ARR_AT(vm->ds, idx);

    memmove(
        &ARR_AT(vm->ds, idx),
        &ARR_AT(vm->ds, idx + 1),
        ( (vm->ds.len - 1) - idx ) * sizeof(word_t)
    );

    vm->ds.len--;
    DISPATCH();
} 

#define BASIC_ARITH(name, oper)                                                \
  op_##name : vm->tos = vm->tos oper ARR_POP(vm->ds);                          \
  DISPATCH();

  BASIC_ARITH(add, +)
  BASIC_ARITH(sub, -)
  BASIC_ARITH(mul, *)
  BASIC_ARITH(div, /)
  BASIC_ARITH(mod, %)

op_dot:
  printf("%td\n", (sword_t)vm->tos);
  DROP();
  DISPATCH();

op_dot_s:
  for (size_t i = 1; i < vm->ds.len; i++) {
    printf("%td ", (sword_t)vm->ds.data[i]);
  }

  if (vm->ds.len)
    printf("%td ", (sword_t)vm->tos);

  printf("\n");
  DISPATCH();

op_call: {
  word_t p = 0;
  memcpy(&p, ++code, sizeof(word_t));
  code += sizeof(word_t);

  ARR_PUSH(vm->rs, (word_t)(code));
  code = (code_t *)p;
  DISPATCH_STAY();
}

op_call_dyn: {
  code = (code_t *)vm->tos;
  DROP();
  DISPATCH_STAY();
}

op_word_call_ptr: {
  const Word *f = (const Word *)vm->tos;
  vm->tos = (word_t)f->code.data;
  DISPATCH();
}

op_compile_code: {
  const Word *f = (const Word *)vm->tos;
  DROP();
  compile_later(&vm->comp, f);
  DISPATCH();
}

op_next_token: {
  TextStream tok = next_token(&vm->input);
  size_t len = tok.end - tok.start;
  PUSH((word_t)tok.start);
  PUSH((word_t)len);
  DISPATCH();
}

op_find_word: {
  const char *text = (void *)ARR_POP(vm->ds);
  size_t len = (size_t)vm->tos;
  vm->tos = (word_t)lex_find(vm->lex, text, len);
  DISPATCH();
}

// ---------- Function Start ----------

#define OP_FUNC_START(name, EXTRA)                                             \
  name : {                                                                     \
    TextStream tok = next_token(&vm->input);                                   \
    LexEntry *entry = make_lex_entry(tok.start, tok.end - tok.start);          \
    EXTRA                                                                      \
    PUSH((word_t)entry);                                                       \
    PUSH(0);                                                                   \
    DISPATCH_CALL(code_compile_loop);                                          \
  }

  OP_FUNC_START(op_func_start,
                /* nothing */
  )

  OP_FUNC_START(op_now_func_start, entry->word.is_now = true;)

  // ---------- Function End Core ----------

#define FUNC_END_COMMON(EXTRA)                                                 \
  word_t tag = vm->tos;                                                        \
  if (tag != 0) {                                                              \
    printf("wrong tag in return statment\n");                                  \
    return;                                                                    \
  }                                                                            \
  DROP();                                                                      \
  LexEntry *entry = (LexEntry *)vm->tos;                                       \
  DROP();                                                                      \
                                                                               \
  ARR_PUSH(vm->comp, OP_RET);                                                  \
  entry->word.code = take_comp(vm);                                            \
                                                                               \
  EXTRA                                                                        \
                                                                               \
  ASSERT(vm->lex);                                                             \
  lex_insert_entry(vm->lex, entry);                                            \
                                                                               \
  (void)ARR_POP(vm->rs);                                                       \
  goto op_ret;

  // ---------- Function End Variants ----------

#define OP_FUNC_END(name, EXTRA)                                               \
  name : { FUNC_END_COMMON(EXTRA) }

  OP_FUNC_END(op_func_outline_end,
              /* nothing */
  )

  OP_FUNC_END(op_func_inline_end, entry->word.is_inline = true;)

  OP_FUNC_END(op_func_end, if (entry->word.code.len <= sizeof(word_t) + 1)
                               entry->word.is_inline = true;)

op_compile_loop: {
  // puts("runing compile loop");

  TextStream tok = next_token(&vm->input);
  if (tok.start == tok.end)
    DISPATCH();
  const char *text = tok.start;
  size_t len = tok.end - tok.start;

  const Word *w = lex_find(vm->lex, text, len);

  if (w) {
    if (w->is_now) {
      DISPATCH_LOOPING_CALL(w->code.data);
    } else {
      compile_later(&vm->comp, w);
      goto op_compile_loop;
    }
  }

  word_t num = 0;
  if (parse_number(&num, text, len)) {
    fprintf(stderr, "error: unrecognized token '%.*s'\n", (int)len, text);
    return;
  };

  comp_push_code(&vm->comp, OP_PUSH_CONST);
  comp_push_word(&vm->comp, num);
  goto op_compile_loop;
}

op_run_loop: {
  // puts("runing eval loop");

  TextStream tok = next_token(&vm->input);
  if (tok.start == tok.end)
    DISPATCH();
  const char *text = tok.start;
  size_t len = tok.end - tok.start;

  const Word *w = lex_find(vm->lex, text, len);
  if (w) {
    DISPATCH_LOOPING_CALL(w->code.data);
  }

  word_t num = 0;
  if (parse_number(&num, text, len)) {
    fprintf(stderr, "error: unrecognized token '%.*s'\n", (int)len, text);
    return;
  };
  PUSH(num);
  goto op_run_loop;
}
}
