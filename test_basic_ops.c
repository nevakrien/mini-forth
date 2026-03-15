#include "vm.h"
#include "compile.h"
#include "basic_ops.h"
#include <stdio.h>
#include <string.h>

int main(void){
	VM vm = {0};

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

	printf("Result: %lu\n", (unsigned long)vm.tos);

	return 0;
}
