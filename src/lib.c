#include "lib.h"

#define LOGGER_IMPLEMENTATION
#include "logger.h"
#define TOKEN_IMPLEMENTATION
#include "lexer/lexer.h"
#define NODE_IMPLEMENTATION
#include "parser/parser.h"
#include "parser/ast_printer.h"
#include "type_checker/checker.h"
#include "code_generator/generator.h"
#include "code_generator/disassembler.h"
#include "interpreter/interpreter.h"
#include "jit_compiler/jit.h"
#include "transpiler/c_transpiler.h"

#include "str.h"

#include <stdio.h>


int c_transpile(Str name, Str source, int verbose) {
    TokenArray array = lexer_lex(name, source);
    if (array.tokens == NULL) {
        error(0, "Failed to lex source\n");
        return -1;
    }

    GrammarTree grammar_tree = parse(array);
    if (grammar_tree.nodes == NULL) {
        error(0, "Failed to parse source\n");
        return -1;
    }

    if (verbose)
        ast_print(grammar_tree, stdout);

    TypedAst typed_tree = type_check(grammar_tree);
    if (typed_tree.nodes == NULL) {
        error(0, "Failed to type check source\n");
        return -1;
    }

    Bytecode code = generate_code(typed_tree);
    if (code.instructions == NULL) {
        error(0, "Failed to generate code\n");
        return -1;
    }

    compile_c(code);

    return 0;
}


InterpreterResult run(Str name, Str source, int verbose) {
    TokenArray array = lexer_lex(name, source);
    if (!token_array_is_ok(array)) {
        error(0, "Failed to lex source\n");
        return (InterpreterResult) { 0, 1 };
    }

    GrammarTree grammar_tree = parse(array);
    if (!grammar_tree_is_ok(grammar_tree)) {
        error(0, "Failed to parse source\n");
        return (InterpreterResult) { 0, 1 };
    }

    if (verbose) {
        ast_print(grammar_tree, stdout);
    }

    TypedAst typed_tree = type_check(grammar_tree);
    if (!typed_ast_is_ok(typed_tree)) {
        error(0, "Failed to type check source\n");
        return (InterpreterResult) { 0, 1 };
    }

    Bytecode code = generate_code(typed_tree);
    if (!bytecode_is_ok(code)) {
        error(0, "Failed to generate code\n");
        return (InterpreterResult) { 0, 1 };
    }

    if (verbose) {
        info(0, "Bytecode:");
        disassemble(code, stdout);
    }

    JitFunction jitted_function = jit_compile(code, verbose);
    if (jitted_function) {
        i64 result = jitted_function();
        return (InterpreterResult) { result, 0 };
    } else {
        warn(0, "Failed to JIT compile");
    }

    i64 result = interpret(code);
    return (InterpreterResult) { result, 0 };
}


i64 repl(void) {
    logger_set_level(LOG_LEVEL_ASSERT);

    char source[4096] = { 0 };
    char buffer[64] = { 0 };
    size_t source_length = 0;

    while (1) {
        printf("> ");
        fgets(buffer, sizeof(buffer), stdin);

        if (strcmp(buffer, "\\exit\n") == 0)
            break;

        size_t length = strlen(buffer);
        memccpy(source + source_length, buffer, '\0', length);
        source_length += length;

        memset(buffer, 0, length);

        InterpreterResult result = run(STR("<repl>"), (Str) { source_length, source }, 0);
        if (result.error) {
            source_length -= length;
            source[source_length] = '\0';
        } else {
            printf("%lld\n", result.result);
        }
    }

    return 0;
}

