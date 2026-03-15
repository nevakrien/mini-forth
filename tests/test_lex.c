#include "vm.h"
#include "compile.h"
#include "basic_ops.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void test_lex_last_version_first_large(void){
	Lex lex;
	lex_init(&lex);

	const size_t count = 10000;
	Word* last_inserted[100];

	for(size_t i = 0; i < count; i++){
		char name[32];
		snprintf(name, sizeof(name), "key_%zu", i % 100);

		Word* word = lex_define(&lex, name, strlen(name));
		word->comp.data = (code_t*)xmalloc(1);
		word->comp.data[0] = (code_t)i;
		word->comp.len = 1;
		word->is_inline = false;
		word->is_now = false;

		if(i >= count - 100){
			last_inserted[i - (count - 100)] = word;
		}	
	}

	for(size_t i = 0; i < 100; i++){
		char name[32];
		snprintf(name, sizeof(name), "key_%zu", i);

		Word* found = lex_find(&lex, name, strlen(name));
		assert(found != NULL && "key should be found");

		assert(found == last_inserted[i] && "last inserted version should be found first (pointer equality)");
		// printf("key_%zu: found pointer %p == expected %p (value=%d)\n", 
			// i, (void*)found, (void*)last_inserted[i], (int)found->comp.data[0]);
	}

	printf("test_lex_last_version_first_large: PASSED (count=%zu)\n", count);

	lex_free(&lex);
}

int main(void){
	test_lex_last_version_first_large();
	printf("\nAll tests PASSED!\n");
	return 0;
}
