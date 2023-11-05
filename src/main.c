#include <stdio.h>

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "type_checker/checker.h"
#include "code_generator/generator.h"
#include "interpreter/interpreter.h"
#include "jit_compiler/jit.h"

#include "str.h"
#include "file.h"


int main(void) {

    Str source = read_file("examples/adding.nox");
    if (source.data == NULL) {
        fprintf(stderr, "Failed to read file\n");
        return 1;
    }

    TokenArray array = lexer_lex(source.data);
    if (array.tokens == NULL) {
        fprintf(stderr, "Failed to lex source\n");
        return 1;
    }

    UntypedAst grammar_tree = parse(array);
    if (grammar_tree.nodes == NULL) {
        fprintf(stderr, "Failed to parse source\n");
        return 1;
    }

    TypedAst typed_tree = type_check(grammar_tree);
    if (typed_tree.nodes == NULL) {
        fprintf(stderr, "Failed to type check source\n");
        return 1;
    }

    Bytecode code = generate_code(typed_tree);
    if (code.instructions == NULL) {
        fprintf(stderr, "Failed to generate code\n");
        return 1;
    }

    JitFunction jitted_function = jit_compile(code);
    if (jitted_function) {
        u64 result = jitted_function();
        printf("[INFO]: JIT result: %llu\n", result);
    } else {
        printf("[INFO]: Failed to JIT compile\n");
    }

    i64 result = interpret(code);
    printf("[INFO]: Interpreter result: %lld\n", result);

    return 0;
}
