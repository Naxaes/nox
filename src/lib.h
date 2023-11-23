#include "lexer/lexer.h"
#include "parser/parser.h"
#include "parser/ast_printer.h"
#include "type_checker/checker.h"
#include "code_generator/generator.h"
#include "code_generator/disassembler.h"
#include "interpreter/interpreter.h"
#include "jit_compiler/jit.h"
#include "transpiler/c_transpiler.h"

#include "logger.h"
#include "str.h"
#include "file.h"
#include "args.h"

#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <locale.h>


int c_transpile_from_source(Str name, Str source, Logger* logger);

Bytecode compile_from_file(Str path, Logger* logger);
Bytecode compile_from_source(Str name, Str source, Logger* logger);

InterpreterResult run_from_file(Str path, Logger* logger);
InterpreterResult run_from_source(Str name, Str source, Logger* logger);

i64 repl(void);
