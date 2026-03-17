#include "vm.h"
#include "compile.h"
#include "basic_ops.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

static void test_add(void){
	VM vm;
	vm_init(&vm);

	comp_push_code(&vm.comp, OP_PUSH_CONST);
	comp_push_word(&vm.comp, 10);
	comp_push_code(&vm.comp, OP_PUSH_CONST);
	comp_push_word(&vm.comp, 20);
	comp_push_code(&vm.comp, OP_ADD);
	comp_push_code(&vm.comp, OP_DONE);

	run_vm(&vm, vm.comp.data);

	assert(vm.tos == 30 && "10 + 20 == 30");
	printf("test_add: PASSED (30)\n");

	vm_free(&vm);
}

static void test_mul(void){
	VM vm;
	vm_init(&vm);

	comp_push_code(&vm.comp, OP_PUSH_CONST);
	comp_push_word(&vm.comp, 10);
	comp_push_code(&vm.comp, OP_PUSH_CONST);
	comp_push_word(&vm.comp, 20);
	comp_push_code(&vm.comp, OP_MUL);
	comp_push_code(&vm.comp, OP_DONE);

	run_vm(&vm, vm.comp.data);

	assert(vm.tos == 200 && "10 * 20 == 200");
	printf("test_mul: PASSED (200)\n");

	vm_free(&vm);
}

static void test_compound(void){
	VM vm;
	vm_init(&vm);

	comp_push_code(&vm.comp, OP_PUSH_CONST);
	comp_push_word(&vm.comp, 10);
	comp_push_code(&vm.comp, OP_PUSH_CONST);
	comp_push_word(&vm.comp, 20);
	comp_push_code(&vm.comp, OP_ADD);
	comp_push_code(&vm.comp, OP_PUSH_CONST);
	comp_push_word(&vm.comp, 5);
	comp_push_code(&vm.comp, OP_MUL);
	comp_push_code(&vm.comp, OP_DONE);

	run_vm(&vm, vm.comp.data);

	printf("test_compound: PASSED (%lu)\n", (unsigned long)vm.tos);

	vm_free(&vm);
}

static void test_dup_drop(void){
	VM vm;
	vm_init(&vm);

	comp_push_code(&vm.comp, OP_PUSH_CONST);
	comp_push_word(&vm.comp, 42);
	comp_push_code(&vm.comp, OP_DUP);
	comp_push_code(&vm.comp, OP_ADD);
	comp_push_code(&vm.comp, OP_DONE);

	run_vm(&vm, vm.comp.data);

	assert(vm.tos == 84 && "42 + 42 == 84");
	printf("test_dup_drop: PASSED (84)\n");

	vm_free(&vm);
}

static void test_sub(void){
	VM vm;
	vm_init(&vm);

	comp_push_code(&vm.comp, OP_PUSH_CONST);
	comp_push_word(&vm.comp, 100);
	comp_push_code(&vm.comp, OP_PUSH_CONST);
	comp_push_word(&vm.comp, 30);
	comp_push_code(&vm.comp, OP_SUB);
	comp_push_code(&vm.comp, OP_DONE);

	run_vm(&vm, vm.comp.data);

	assert((word_t)70 == vm.tos && "100 - 30  == 70");
	printf("test_sub: PASSED (70)\n");

	vm_free(&vm);
}

static void test_call(void){
	VM vm;
	vm_init(&vm);

	Comp func_comp = {0};
	comp_push_code(&func_comp, OP_PUSH_CONST);
	comp_push_word(&func_comp, 10);
	comp_push_code(&func_comp, OP_PUSH_CONST);
	comp_push_word(&func_comp, 20);
	comp_push_code(&func_comp, OP_ADD);
	comp_push_code(&func_comp, OP_RET);

	comp_push_code(&vm.comp, OP_PUSH_CONST);
	comp_push_word(&vm.comp, 5);
	comp_push_code(&vm.comp, OP_CALL);
	comp_push_word(&vm.comp, (word_t)(func_comp.data));
	comp_push_code(&vm.comp, OP_DONE);

	run_vm(&vm, vm.comp.data);

	assert(vm.tos == 30 && "5 + (10 + 20) == 30");
	printf("test_call: PASSED (30)\n");

	free(func_comp.data);
	vm_free(&vm);
}

static void test_run_loop(){
	Lex lex;
    lex_init(&lex);

    TextStream text ={0};
    text.start = "1 2 +";
    text.end = text.start+strlen(text.start);

    VM vm;
	vm_init(&vm);

	vm.lex=&lex;
	vm.input=text;
	run_text(&vm);
	assert(vm.tos==3);
	printf("test_run_loop: PASSED (3)\n");

	vm_free(&vm);
    lex_free(&lex);
}

static void test_end_to_end(){
	Lex lex;
    lex_init(&lex);

    TextStream text ={0};
    text.start = "2 : add1 1 + ; add1";
    text.end = text.start+strlen(text.start);

    VM vm;
	vm_init(&vm);

	vm.lex=&lex;
	vm.input=text;
	run_text(&vm);
	assert(vm.tos==3);
	printf("test_end_to_end: PASSED (3)\n");

	vm_free(&vm);
    lex_free(&lex);
}

static void test_run_stop_codes(void){
	Lex lex;
    lex_init(&lex);

	{
		VM vm;
		vm_init(&vm);
		vm.lex = &lex;

		TextStream text = {0};
		text.start = "1 2 +";
		text.end = text.start + strlen(text.start);
		vm.input = text;

		assert(run_text(&vm) == STOP_REASON_EVAL_INPUT_EMPTY);
		vm_free(&vm);
	}

	{
		VM vm;
		vm_init(&vm);
		vm.lex = &lex;

		TextStream text = {0};
		text.start = "bye";
		text.end = text.start + strlen(text.start);
		vm.input = text;

		assert(run_text(&vm) == STOP_REASON_BYE);
		vm_free(&vm);
	}

	{
		VM vm;
		vm_init(&vm);
		vm.lex = &lex;

		TextStream text = {0};
		text.start = ": add1 1 +";
		text.end = text.start + strlen(text.start);
		vm.input = text;

		assert(run_text(&vm) == STOP_REASON_COMPILE_INPUT_EMPTY);

		text.start = "; 2 add1";
		text.end = text.start + strlen(text.start);
		vm.input = text;

		assert(run_compile_text(&vm) == STOP_REASON_EVAL_INPUT_EMPTY);
		assert(vm.tos == 3);
		vm_free(&vm);
	}

	{
		VM vm;
		vm_init(&vm);
		vm.lex = &lex;

		TextStream text = {0};
		text.start = "wat";
		text.end = text.start + strlen(text.start);
		vm.input = text;

		assert(run_text(&vm) == STOP_REASON_ERROR);
		vm_free(&vm);
	}

	printf("test_run_stop_codes: PASSED\n");
	lex_free(&lex);
}

static void test_pick_roll(void){
	VM vm;
	vm_init(&vm);

	// start: 1 2 3 4
	comp_push_code(&vm.comp, OP_PUSH_CONST);
	comp_push_word(&vm.comp, 1);
	comp_push_code(&vm.comp, OP_PUSH_CONST);
	comp_push_word(&vm.comp, 2);
	comp_push_code(&vm.comp, OP_PUSH_CONST);
	comp_push_word(&vm.comp, 3);
	comp_push_code(&vm.comp, OP_PUSH_CONST);
	comp_push_word(&vm.comp, 4);

	// 2 pick
	// stack before pick (after consuming index): 1 2 3 4
	// result: 1 2 3 4 2
	comp_push_code(&vm.comp, OP_PUSH_CONST);
	comp_push_word(&vm.comp, 2);
	comp_push_code(&vm.comp, OP_PICK);
	

	// 3 roll
	// stack before roll (after consuming index): 1 2 3 4 2
	// result: 1 3 4 2 2
	comp_push_code(&vm.comp, OP_PUSH_CONST);
	comp_push_word(&vm.comp, 3);
	comp_push_code(&vm.comp, OP_ROLL);

	comp_push_code(&vm.comp, OP_DONE);

	run_vm(&vm, vm.comp.data);

	assert(vm.ds.len == 5);

	// final logical stack is: 1 3 4 2 2
	// so popping from top should give: 2, 2, 4, 3, 1

	assert(vm.tos == 2);
	vm.tos = ARR_POP(vm.ds);

	assert(vm.tos == 2);
	vm.tos = ARR_POP(vm.ds);

	assert(vm.tos == 4);
	vm.tos = ARR_POP(vm.ds);

	assert(vm.tos == 3);
	vm.tos = ARR_POP(vm.ds);

	assert(vm.tos == 1);

	printf("test_pick_roll: PASSED\n");

	vm_free(&vm);
}

#ifndef VM_HARD_STACK_ERRORS
static void assert_run_vm_stop_reason(Comp *comp,
				      StopReason expected,
				      const char *test_name)
{
	printf("%s: ", test_name);
	fflush(stdout);

	VM vm;
	vm_init(&vm);

	StopReason actual = run_vm(&vm, comp->data);
	assert(actual == expected);

	printf("PASSED\n");
	vm_free(&vm);
}

static void test_stack_error_paths(void){
	{
		Comp comp = {0};
		comp_push_code(&comp, OP_DROP);
		comp_push_code(&comp, OP_DONE);
		assert_run_vm_stop_reason(&comp, STOP_REASON_ERROR, "test_drop_underflow");
		free(comp.data);
	}

	{
		Comp comp = {0};
		comp_push_code(&comp, OP_PUSH_CONST);
		comp_push_word(&comp, 1);
		comp_push_code(&comp, OP_ADD);
		comp_push_code(&comp, OP_DONE);
		assert_run_vm_stop_reason(&comp, STOP_REASON_ERROR, "test_add_underflow");
		free(comp.data);
	}

	{
		Comp comp = {0};
		comp_push_code(&comp, OP_PUSH_CONST);
		comp_push_word(&comp, 1);
		comp_push_code(&comp, OP_OVER);
		comp_push_code(&comp, OP_DONE);
		assert_run_vm_stop_reason(&comp, STOP_REASON_ERROR, "test_over_underflow");
		free(comp.data);
	}

	{
		Comp comp = {0};
		comp_push_code(&comp, OP_PUSH_CONST);
		comp_push_word(&comp, 1);
		comp_push_code(&comp, OP_PUSH_CONST);
		comp_push_word(&comp, 5);
		comp_push_code(&comp, OP_PICK);
		comp_push_code(&comp, OP_DONE);
		assert_run_vm_stop_reason(&comp, STOP_REASON_ERROR, "test_pick_depth_oob");
		free(comp.data);
	}

	{
		Comp comp = {0};
		comp_push_code(&comp, OP_PUSH_CONST);
		comp_push_word(&comp, 1);
		comp_push_code(&comp, OP_PUSH_CONST);
		comp_push_word(&comp, 5);
		comp_push_code(&comp, OP_ROLL);
		comp_push_code(&comp, OP_DONE);
		assert_run_vm_stop_reason(&comp, STOP_REASON_ERROR, "test_roll_depth_oob");
		free(comp.data);
	}

	{
		Comp comp = {0};
		comp_push_code(&comp, OP_POP_RS);
		comp_push_code(&comp, OP_DONE);
		assert_run_vm_stop_reason(&comp, STOP_REASON_ERROR, "test_pop_rs_underflow");
		free(comp.data);
	}

	{
		Comp comp = {0};
		comp_push_code(&comp, OP_RET);
		assert_run_vm_stop_reason(&comp, STOP_REASON_ERROR, "test_ret_underflow");
		free(comp.data);
	}

	{
		Comp comp = {0};
		comp_push_code(&comp, OP_PUSH_RS);
		comp_push_code(&comp, OP_DONE);
		assert_run_vm_stop_reason(&comp, STOP_REASON_ERROR, "test_push_rs_underflow");
		free(comp.data);
	}
}
#endif

static void assert_source_stack(const char *src,
                                const word_t *expected,
                                size_t expected_len,
                                const char *test_name)
{
    printf("%s: ", test_name);
    fflush(stdout);

    Lex lex;
    lex_init(&lex);

    TextStream text = {0};
    text.start = src;
    text.end   = src + strlen(src);

    VM vm;
    vm_init(&vm);

    vm.lex   = &lex;
    vm.input = text;

    run_text(&vm);

    size_t actual_len = vm.ds.len;

    if (actual_len != expected_len) {
        printf("FAILED\n");
        fprintf(stderr,
                "%s: stack length mismatch (expected %zu got %zu)\n",
                test_name, expected_len, actual_len);

        fprintf(stderr, "actual stack: ");
        for (size_t i = 1; i < vm.ds.len; i++)
            fprintf(stderr, "%td ", (sword_t)vm.ds.data[i]);

        if (vm.ds.len)
            fprintf(stderr, "%td ", (sword_t)vm.tos);

        fprintf(stderr, "\n");

        assert(0);
    }

    size_t idx = 0;

    for (size_t i = 1; i < vm.ds.len; i++, idx++) {
        if (vm.ds.data[i] != expected[idx]) {
            printf("FAILED\n");
            fprintf(stderr,
                    "%s: mismatch at %zu (expected %td got %td)\n",
                    test_name,
                    idx,
                    (sword_t)expected[idx],
                    (sword_t)vm.ds.data[i]);
            assert(0);
        }
    }

    if (vm.ds.len) {
        if (vm.tos != expected[idx]) {
            printf("FAILED\n");
            fprintf(stderr,
                    "%s: mismatch at %zu (expected %td got %td)\n",
                    test_name,
                    idx,
                    (sword_t)expected[idx],
                    (sword_t)vm.tos);
            assert(0);
        }
    }

    printf("PASSED\n");

    vm_free(&vm);
    lex_free(&lex);
}

static void test_stack_words_source(void) {
	{
		const word_t expected[] = {2, 1};
		assert_source_stack("1 2 swap", expected, 2, "test_swap_source");
	}
	{
		const word_t expected[] = {1, 2, 1};
		assert_source_stack("1 2 over", expected, 3, "test_over_source");
	}
	{
		const word_t expected[] = {2};
		assert_source_stack("1 2 nip", expected, 1, "test_nip_source");
	}
	{
		const word_t expected[] = {2, 1, 2};
		assert_source_stack("1 2 tuck", expected, 3, "test_tuck_source");
	}
	{
		const word_t expected[] = {2, 3, 1};
		assert_source_stack("1 2 3 rot", expected, 3, "test_rot_source");
	}
	{
		const word_t expected[] = {3, 1, 2};
		assert_source_stack("1 2 3 -rot", expected, 3, "test_nrot_source");
	}
	{
		const word_t expected[] = {1, 2, 1, 2};
		assert_source_stack("1 2 2dup", expected, 4, "test_2dup_source");
	}
	{
		assert_source_stack("1 2 2drop", NULL, 0, "test_2drop_source");
	}
	{
		const word_t expected[] = {3, 4, 1, 2};
		assert_source_stack("1 2 3 4 2swap", expected, 4, "test_2swap_source");
	}
	{
		const word_t expected[] = {1, 2, 3, 4, 1, 2};
		assert_source_stack("1 2 3 4 2over", expected, 6, "test_2over_source");
	}
}

static void test_comparison_words_source(void) {
	{
		const word_t expected[] = {1};
		assert_source_stack("5 5 =", expected, 1, "test_eq_source");
	}
	{
		const word_t expected[] = {1};
		assert_source_stack("5 7 ~=", expected, 1, "test_ne_source");
	}
	{
		const word_t expected[] = {0};
		assert_source_stack("5 2 <", expected, 1, "test_lt_source");
	}
	{
		const word_t expected[] = {0};
		assert_source_stack("2 5 >", expected, 1, "test_gt_source");
	}
	{
		const word_t expected[] = {1};
		assert_source_stack("5 5 <=", expected, 1, "test_le_source");
	}
	{
		const word_t expected[] = {1};
		assert_source_stack("5 5 >=", expected, 1, "test_ge_source");
	}
	{
		const word_t expected[] = {1};
		assert_source_stack("0 0=", expected, 1, "test_0eq_source");
	}
	{
		const word_t expected[] = {1};
		assert_source_stack("7 0~=", expected, 1, "test_0ne_source");
	}
}

static void test_bitwise_words_source(void) {
	{
		const word_t expected[] = {1};
		assert_source_stack("5 3 &", expected, 1, "test_and_source");
	}
	{
		const word_t expected[] = {7};
		assert_source_stack("5 3 |", expected, 1, "test_or_source");
	}
	{
		const word_t expected[] = {6};
		assert_source_stack("5 3 ^", expected, 1, "test_xor_source");
	}
	{
		const word_t expected[] = {(word_t)~(word_t)5};
		assert_source_stack("5 ~", expected, 1, "test_not_source");
	}
	{
		const word_t expected[] = {8};
		assert_source_stack("1 3 <<", expected, 1, "test_shl_source");
	}
	{
		const word_t expected[] = {4};
		assert_source_stack("32 3 >>", expected, 1, "test_shr_source");
	}
}

static void test_if_words_source(void) {
	{
		const word_t expected[] = {7};
		assert_source_stack(": choose 1 if 7 end ; choose",
		                    expected, 1, "test_if_true_source");
	}
	{
		assert_source_stack(": choose 0 if 7 end ; choose",
		                    NULL, 0, "test_if_false_source");
	}
	{
		const word_t expected[] = {7};
		assert_source_stack(": choose 1 if 7 else 9 end ; choose",
		                    expected, 1, "test_if_else_true_source");
	}
	{
		const word_t expected[] = {9};
		assert_source_stack(": choose 0 if 7 else 9 end ; choose",
		                    expected, 1, "test_if_else_false_source");
	}
}

static void test_begin_words_source(void) {
	{
		const word_t expected[] = {120};
		assert_source_stack(
		    ": fact 1 swap begin dup 1 <= if drop raw-ret end tuck * swap 1 - end ;outline 5 fact",
		    expected, 1, "test_begin_factorial_source");
	}
}

int main(void){
	test_add();
	test_mul();
	test_compound();
	test_dup_drop();
	test_sub();
	test_call();
	test_run_loop();
	test_end_to_end();
	test_run_stop_codes();
	test_pick_roll();
#ifndef VM_HARD_STACK_ERRORS
	test_stack_error_paths();
#endif
	test_stack_words_source();
	test_comparison_words_source();
	test_bitwise_words_source();
	test_if_words_source();
	test_begin_words_source();
	
	printf("\nAll tests PASSED!\n");
	return 0;
}
