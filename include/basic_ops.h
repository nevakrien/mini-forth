#ifndef BASIC_OPS_H
#define BASIC_OPS_H

#include "vm.h"
#include <stddef.h>
#include <string.h>

typedef enum {
  STOP_REASON_BYE = 0,
  STOP_REASON_EVAL_INPUT_EMPTY,
  STOP_REASON_COMPILE_INPUT_EMPTY,
  STOP_REASON_ERROR,
} StopReason;

StopReason run_vm(VM* vm, const code_t* code);

#endif // BASIC_OPS_H
