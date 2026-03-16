#include "vm.h"
#include "compile.h"
#include "basic_ops.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_BUF_SIZE 4096

int main(void){
    VM vm;
    vm_init(&vm);
    Lex lex;
    lex_init(&lex);
    vm.lex = &lex;

    char line[LINE_BUF_SIZE];

    printf("Stack REPL - type 'bye' to exit\n");

    while(1){
        printf("> ");
        if(!fgets(line, sizeof(line), stdin)){
            break;
        }

        size_t len = strlen(line);
        if(len > 0 && line[len-1] == '\n'){
            line[len-1] = '\0';
            len--;
        }

        if(len == 0){
            continue;
        }

        if(run_text(&vm, line, line + len)){
            printf("Error running: %s\n", line);
        }
    }

    lex_free(&lex);
    vm_free(&vm);
    return 0;
}
