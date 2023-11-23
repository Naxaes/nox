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




int c_transpile_from_source(Str name, Str source, Logger* logger) {
    TokenArray array = lexer_lex(name, source, logger);
    if (array.tokens == NULL) {
        error(logger, "Failed to lex source\n");
        return -1;
    }

    GrammarTree grammar_tree = parse(array, logger);
    if (grammar_tree.nodes == NULL) {
        error(logger, "Failed to parse source\n");
        return -1;
    }

    if (logger->level == LOG_LEVEL_DEBUG)
        ast_print(grammar_tree, stdout);

    TypedAst ast = type_check(grammar_tree, logger);
    if (ast.tree.nodes == NULL) {
        error(logger, "Failed to type check source\n");
        return -1;
    }

    Bytecode code = generate_code(ast);
    if (code.instructions == NULL) {
        error(logger, "Failed to generate code\n");
        return -1;
    }

    compile_c(code);

    return 0;
}




Bytecode compile_from_source(Str name, Str source, Logger* logger) {
    debug(logger, "Source %s:\n%s\n", name.data, source.data);

    TokenArray array = lexer_lex(name, source, logger);
    if (array.tokens == NULL) {
        error(logger, "Failed to lex source\n");
        return (Bytecode) { 0, 0 };
    }

    GrammarTree grammar_tree = parse(array, logger);
    if (grammar_tree.nodes == NULL) {
        error(logger, "Failed to parse source\n");
        return (Bytecode) { 0, 0 };
    }

    if (logger->level == LOG_LEVEL_DEBUG)
        ast_print(grammar_tree, stdout);

    TypedAst ast = type_check(grammar_tree, logger);
    if (ast.tree.nodes == NULL) {
        error(logger, "Failed to type check source\n");
        return (Bytecode) { 0, 0 };
    }

    Bytecode code = generate_code(ast);
    return code;
}

InterpreterResult run_from_source(Str name, Str source, Logger* logger) {
    Bytecode code = compile_from_source(name, source, logger);

    if (code.instructions == NULL) {
        error(logger, "Failed to generate code\n");
        return (InterpreterResult) { 0, 1 };
    }

    if (logger->level == LOG_LEVEL_DEBUG) {
        fprintf(stdout, "\nBytecode:\n");
        disassemble(code, stdout);
        printf("\n");
    }

    JittedFunction jitted_function = jit_compile(code, 1);
    if (jitted_function.function) {
        i64 result = jitted_function.function();
        jit_free(jitted_function);
        return (InterpreterResult) { result, 0 };
    } else {
        warn(logger, "[INFO]: Failed to JIT compile\n");
    }

    InterpreterResult result = interpret(code);

    dealloc(code.instructions);
    return result;
}


Bytecode compile_from_file(Str path, Logger* logger) {
    Str source = read_file(path.data);

    if (str_is_empty(source)) {
        error(logger, "Failed to read file\n");
        return (Bytecode) { 0, 0 };
    }

    Bytecode result = compile_from_source(path, source, logger);
    dealloc((char*) source.data);
    return result;
}

InterpreterResult run_from_file(Str path, Logger* logger) {
    Str source = read_file(path.data);

    if (str_is_empty(source)) {
        error(logger, "Failed to read file\n");
        return (InterpreterResult) { 0, 1 };
    }

    InterpreterResult result = run_from_source(path, source, logger);
    dealloc((char*) source.data);
    return result;
}


i64 repl(void) {
    char source[4096] = { 0 };
    char buffer[64] = { 0 };
    size_t source_length = 0;

    Logger logger = logger_make_with_file("REPL", LOG_LEVEL_ERROR, stdout);

    while (1) {
        printf("> ");
        fgets(buffer, sizeof(buffer), stdin);

        size_t length = strlen(buffer);
        memccpy(source + source_length, buffer, '\0', length);
        source_length += length;

        memset(buffer, 0, length);

        InterpreterResult result = run_from_source(STR("<repl>"), (Str) { source_length, source }, &logger);
        if (result.error) {
            source_length -= length;
            source[source_length] = '\0';
        } else {
            printf("%lld\n", result.result);
        }
    }

    return 0;
}
