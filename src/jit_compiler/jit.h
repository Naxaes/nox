#pragma once

#include "code_generator/generator.h"


typedef i64 (*JitFunction)(void);


JitFunction jit_compile(Bytecode code);
