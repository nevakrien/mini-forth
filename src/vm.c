#include "vm.h"
#include <stdlib.h>

void vm_init(VM* vm){
	*vm = (VM){0};
}

void vm_free(VM* vm){
	free(vm->ds.data);
	free(vm->rs.data);
	free(vm->comp.data);
	*vm = (VM){0};
}
