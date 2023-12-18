#pragma once

#include "code_generator/generator.h"


typedef i64 (*JitFunction)(void);
typedef struct {
    JitFunction function;
    size_t      size;
} JittedFunction;


void jit_free(JittedFunction function);
JittedFunction jit_compile(Bytecode code, int output);
