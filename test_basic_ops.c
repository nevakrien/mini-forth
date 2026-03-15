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

	assert((word_t)-70 == vm.tos && "30 - 100 == -70");
	printf("test_sub: PASSED (-70)\n");

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

int main(void){
	test_add();
	test_mul();
	test_compound();
	test_dup_drop();
	test_sub();
	test_call();
	
	printf("\nAll tests PASSED!\n");
	return 0;
}
