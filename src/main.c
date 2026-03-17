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

    bool resume_compile = false;

    while(1){
        printf(resume_compile ? "... " : "> ");
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

        vm.input.start=line;
        vm.input.end =line + len;
        StopReason stop = resume_compile ? run_compile_text(&vm) : run_text(&vm);
        if(stop == STOP_REASON_BYE){
            break;
        }

        resume_compile = (stop == STOP_REASON_COMPILE_INPUT_EMPTY);

        if(stop == STOP_REASON_ERROR){
            printf("err\n");
        } else if(stop == STOP_REASON_EVAL_INPUT_EMPTY){
            printf("ok\n");
        }
    }

    lex_free(&lex);
    vm_free(&vm);
    return 0;
}
