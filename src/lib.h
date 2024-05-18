#pragma once

#include "str.h"


typedef struct {
    i64 result;
    int error;
} InterpreterResult;


int c_transpile(Str name, Str source, int verbose);
InterpreterResult run(Str name, Str source, int verbose);
i64 repl(void);
