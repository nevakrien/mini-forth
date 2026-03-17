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


StopReason run_vm(VM *vm, const code_t *code) {
  // puts("run_vm start");

  static const void *const dispatch[] = {
      [OP_DONE] = &&op_done,
      [OP_CRASH] = &&op_crash,
      [OP_PUSH_CONST] = &&op_push_const,
      [OP_CONST_PRINT] = &&op_const_print,
      [OP_COMPILE_CONST_PRINT] = &&op_compile_const_print,
      [OP_CALL] = &&op_call,
      [OP_CALL_DYN] = &&op_call_dyn,
      [OP_RET] = &&op_ret,
      [OP_DROP] = &&op_drop,
      [OP_DUP] = &&op_dup,
      [OP_SWAP] = &&op_swap,
      [OP_OVER] = &&op_over,
      [OP_NIP] = &&op_nip,
      [OP_TUCK] = &&op_tuck,
      [OP_ROT] = &&op_rot,
      [OP_NROT] = &&op_nrot,
      [OP_2DUP] = &&op_2dup,
      [OP_2DROP] = &&op_2drop,
      [OP_2SWAP] = &&op_2swap,
      [OP_2OVER] = &&op_2over,
      [OP_PICK] = &&op_pick,
      [OP_ROLL] = &&op_roll,

      [OP_ADD] = &&op_add,
      [OP_SUB] = &&op_sub,
      [OP_MUL] = &&op_mul,
      [OP_DIV] = &&op_div,
      [OP_MOD] = &&op_mod,
      [OP_EQ] = &&op_eq,
      [OP_NE] = &&op_ne,
      [OP_LT] = &&op_lt,
      [OP_GT] = &&op_gt,
      [OP_LE] = &&op_le,
      [OP_GE] = &&op_ge,
      [OP_ZEQ] = &&op_zeq,
      [OP_ZNE] = &&op_zne,
      [OP_BIT_AND] = &&op_bit_and,
      [OP_BIT_OR] = &&op_bit_or,
      [OP_BIT_XOR] = &&op_bit_xor,
      [OP_BIT_NOT] = &&op_bit_not,
      [OP_SHL] = &&op_shl,
      [OP_SHR] = &&op_shr,
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

      [OP_BRANCH] = &&op_branch,
      [OP_JUMP] = &&op_jump,

      [OP_COMP_IDX] = &&op_comp_idx,
      [OP_COMPILE_JUMP] = &&op_compile_jump,
      [OP_COMPILE_BRANCH] = &&op_compile_branch,
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
  return STOP_REASON_BYE;

op_crash:
  return STOP_REASON_ERROR;

op_push_const: {
  word_t c = 0;
  memcpy(&c, ++code, sizeof(word_t));
  code += sizeof(word_t);
  PUSH(c);
  DISPATCH_STAY();
}

op_const_print: {
    word_t len = 0;

    memcpy(&len, ++code, sizeof(word_t));
    code += sizeof(word_t);

    fwrite((const char *)code, 1, (size_t)len, stdout);

    code += len;

    DISPATCH_STAY();
}

op_compile_const_print: {
    ARR_PUSH(vm->comp, OP_CONST_PRINT);

    size_t len_pos = vm->comp.len;
    comp_push_word(&vm->comp, 0);   // placeholder for length

    size_t start = vm->comp.len;

    //skip the first space
    if(vm->input.start != vm->input.end)
        vm->input.start++;
    
    for (;;) {
        if (vm->input.start == vm->input.end) {
            printf("unclosed delimiter\n");
            return STOP_REASON_ERROR;
        }

        if (*vm->input.start == '"') {
            vm->input.start++;
            break;
        }

        if (*vm->input.start == '\\') {
            if(*++vm->input.start=='n'){
                ARR_PUSH(vm->comp,'\n');
                vm->input.start++;
                continue;
            }
        }

        ARR_PUSH(vm->comp, *vm->input.start++);
    }

    word_t len = vm->comp.len - start;

    memcpy(&ARR_AT(vm->comp, len_pos), &len, sizeof(word_t));

    DISPATCH();
}

op_ret:
  code = (code_t *)ARR_POP(vm->rs);
  DISPATCH_STAY();

op_branch:{
    ASSERT(vm->ds.len >= 1);
    word_t cond= vm->tos;
    if(cond){
        goto op_jump;
    }

    code+=sizeof(boffset_t)+1;
    DISPATCH_STAY();
}

op_comp_idx:
    PUSH(vm->comp.len);
    DISPATCH();

op_jump:{
    boffset_t offset = 0;
    memcpy(&offset,code+1, sizeof(boffset_t));

    code+=offset;
    DISPATCH_STAY();
}

op_compile_jump:{
    //offset always fits in word_t
    boffset_t offset = (boffset_t)vm->tos;
    DROP();

    ARR_PUSH(vm->comp,OP_JUMP);
    comp_push_offset(&vm->comp,offset);
    DISPATCH();
}

op_compile_branch:{
    //offset always fits in word_t
    boffset_t offset = (boffset_t)vm->tos;
    DROP();

    ARR_PUSH(vm->comp,OP_BRANCH);
    comp_push_offset(&vm->comp,offset);
    DISPATCH();
}


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
  ASSERT(vm->ds.len >= 1);
  PUSH(vm->tos);
  DISPATCH();

op_swap: {
  ASSERT(vm->ds.len >= 2);
  word_t top = vm->tos;
  word_t bottom = ARR_PEEK(vm->ds);
  vm->tos = bottom;
  ARR_PEEK(vm->ds)=top;
  DISPATCH();
}

op_over:{
  word_t bot = ARR_PEEK(vm->ds);
  PUSH(bot);
  DISPATCH();
}

op_nip:
  ASSERT(vm->ds.len >= 2);
  (void)ARR_POP(vm->ds);
  DISPATCH();

op_tuck: {
  ASSERT(vm->ds.len >= 2);
  word_t top = vm->tos;
  word_t bottom = ARR_PEEK(vm->ds);
  ARR_PEEK(vm->ds)=top;
  ARR_PUSH(vm->ds, bottom);
  vm->tos = top;
  DISPATCH();
}

op_rot: {
  ASSERT(vm->ds.len >= 3);
  word_t c = vm->tos;
  word_t b = ARR_POP(vm->ds);
  word_t a = ARR_POP(vm->ds);
  ARR_PUSH(vm->ds, b);
  ARR_PUSH(vm->ds, c);
  vm->tos = a;
  DISPATCH();
}

op_nrot: {
  ASSERT(vm->ds.len >= 3);
  word_t c = vm->tos;
  word_t b = ARR_POP(vm->ds);
  word_t a = ARR_POP(vm->ds);
  ARR_PUSH(vm->ds, c);
  ARR_PUSH(vm->ds, a);
  vm->tos = b;
  DISPATCH();
}

op_2dup: {
  ASSERT(vm->ds.len >= 2);
  word_t b = vm->tos;
  word_t a = ARR_PEEK(vm->ds);
  PUSH(a);
  PUSH(b);
  DISPATCH();
}

op_2drop:
  ASSERT(vm->ds.len >= 2);
  DROP();
  DROP();
  DISPATCH();

op_2swap: {
  ASSERT(vm->ds.len >= 4);
  word_t d = vm->tos;
  word_t c = ARR_POP(vm->ds);
  word_t b = ARR_POP(vm->ds);
  word_t a = ARR_POP(vm->ds);
  ARR_PUSH(vm->ds, c);
  ARR_PUSH(vm->ds, d);
  ARR_PUSH(vm->ds, a);
  vm->tos = b;
  DISPATCH();
}

op_2over: {
  ASSERT(vm->ds.len >= 4);
  word_t a = ARR_AT(vm->ds, vm->ds.len - 3);
  word_t b = ARR_AT(vm->ds, vm->ds.len - 2);
  PUSH(a);
  PUSH(b);
  DISPATCH();
}

op_pick: {
    ASSERT(vm->ds.len >= 1);
    word_t depth = vm->tos;
    word_t idx = vm->ds.len - depth-1;
    
    ASSERT(idx!=0);
    vm->tos = ARR_AT(vm->ds, idx);
    
    DISPATCH();
}

op_roll: {
    ASSERT(vm->ds.len >= 1);

    word_t depth = vm->tos;
    word_t idx = vm->ds.len - depth-1;
    
    ASSERT(idx!=0);
    vm->tos = ARR_AT(vm->ds, idx);

    memmove(
        vm->ds.data+idx,
        vm->ds.data+idx+1,
        ( (vm->ds.len - 1) - idx ) * sizeof(word_t)
    );

    vm->ds.len--;
    DISPATCH();
} 

#define BASIC_ARITH(name, oper) \
  op_##name: { \
    ASSERT(vm->ds.len >= 2); \
    word_t rhs = vm->tos; \
    word_t lhs = ARR_POP(vm->ds); \
    vm->tos = (lhs oper rhs); \
    DISPATCH(); \
  }

  BASIC_ARITH(add, +)
  BASIC_ARITH(sub, -)
  BASIC_ARITH(mul, *)
  BASIC_ARITH(div, /)
  BASIC_ARITH(mod, %)
  BASIC_ARITH(bit_and, &)
  BASIC_ARITH(bit_or, |)
  BASIC_ARITH(bit_xor, ^)
  BASIC_ARITH(shl, <<)
  BASIC_ARITH(shr, >>)
  BASIC_ARITH(eq, ==)
  BASIC_ARITH(ne, !=)
  BASIC_ARITH(lt, <)
  BASIC_ARITH(gt, >)
  BASIC_ARITH(le, <=)
  BASIC_ARITH(ge, >=)

op_zeq:
  ASSERT(vm->ds.len >= 1);
  vm->tos = (word_t)(vm->tos == 0);
  DISPATCH();

op_zne:
  ASSERT(vm->ds.len >= 1);
  vm->tos = (word_t)(vm->tos != 0);
  DISPATCH();
op_bit_not:
  ASSERT(vm->ds.len >= 1);
  vm->tos = ~vm->tos;
  DISPATCH();

op_dot:
  ASSERT(vm->ds.len >= 1);

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
  ASSERT(vm->ds.len >= 1);

  code = (code_t *)vm->tos;
  DROP();
  DISPATCH_STAY();
}

op_word_call_ptr: {
  ASSERT(vm->ds.len >= 1);

  const Word *f = (const Word *)vm->tos;
  vm->tos = (word_t)f->code.data;
  DISPATCH();
}

op_compile_code: {
  ASSERT(vm->ds.len >= 1);

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
  ASSERT(vm->ds.len >= 2);

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
    PUSH(COMP_TAG_FUNC);                                                       \
    DISPATCH_CALL(code_compile_loop);                                          \
  }

  OP_FUNC_START(op_func_start,
                /* nothing */
  )

  OP_FUNC_START(op_now_func_start, entry->word.is_now = true;)

  // ---------- Function End Core ----------

#define FUNC_END_COMMON(EXTRA)                                                 \
  ASSERT(vm->ds.len >= 2);                                                    \
  word_t tag = vm->tos;                                                        \
    if (tag != COMP_TAG_FUNC) {                                                  \
      printf("wrong tag in return statment\n");                                  \
      return STOP_REASON_ERROR;                                                  \
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
    return STOP_REASON_COMPILE_INPUT_EMPTY;
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
    return STOP_REASON_ERROR;
  };

  comp_push_code(&vm->comp, OP_PUSH_CONST);
  comp_push_word(&vm->comp, num);
  goto op_compile_loop;
}

op_run_loop: {
  // puts("runing eval loop");

  TextStream tok = next_token(&vm->input);
  if (tok.start == tok.end)
    return STOP_REASON_EVAL_INPUT_EMPTY;
  const char *text = tok.start;
  size_t len = tok.end - tok.start;

  const Word *w = lex_find(vm->lex, text, len);
  if (w) {
    DISPATCH_LOOPING_CALL(w->code.data);
  }

  word_t num = 0;
  if (parse_number(&num, text, len)) {
    fprintf(stderr, "error: unrecognized token '%.*s'\n", (int)len, text);
    return STOP_REASON_ERROR;
  };
  PUSH(num);
  goto op_run_loop;
}
}
