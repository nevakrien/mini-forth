#include "vm.h"
#include "compile.h"
#include "basic_ops.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct {
    Word* ptr;
    code_t expected;
} PtrCheck;

static void test_lex_stress(void) {
    Lex lex;
    lex_init(&lex);

    const size_t inserts = 20000;
    const size_t unique_keys = 500;

    PtrCheck *checks = malloc(sizeof(PtrCheck) * inserts);
    size_t check_len = 0;

    Word *last_seen[unique_keys];
    memset(last_seen, 0, sizeof(last_seen));

    for (size_t i = 0; i < inserts; i++) {
        char name[32];
        size_t k = i % unique_keys;
        snprintf(name, sizeof(name), "key_%zu", k);

        Word* word = lex_define(&lex, name, strlen(name));

        word->comp.data = (code_t*)xmalloc(sizeof(code_t));
        word->comp.data[0] = (code_t)i;
        word->comp.len = 1;
        word->is_inline = false;
        word->is_now = false;

        checks[check_len++] = (PtrCheck){ word, (code_t)i };
        last_seen[k] = word;

        /* periodically verify pointer stability */
        if ((i % 251) == 0) {
            for (size_t j = 0; j < check_len; j++) {
                assert(checks[j].ptr->comp.data[0] == checks[j].expected);
            }
        }
    }

    /* verify duplicate semantics (last definition wins) */
    for (size_t i = 0; i < unique_keys; i++) {
        char name[32];
        snprintf(name, sizeof(name), "key_%zu", i);

        Word* found = lex_find(&lex, name, strlen(name));
        assert(found != NULL);
        assert(found == last_seen[i]);
    }

    /* final pointer stability check */
    for (size_t j = 0; j < check_len; j++) {
        assert(checks[j].ptr->comp.data[0] == checks[j].expected);
    }

    printf("test_lex_stress: PASSED (%zu inserts)\n", inserts);

    free(checks);
    lex_free(&lex);
}

int main(void) {
    test_lex_stress();
    printf("\nAll tests PASSED!\n");
    return 0;
}