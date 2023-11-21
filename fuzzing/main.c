#include "lexer/lexer.h"
#include "parser/parser.h"
#include "type_checker/checker.h"
#include "code_generator/generator.h"
#include "interpreter/interpreter.h"
#include "jit_compiler/jit.h"


int LLVMFuzzerTestOneInput(const u8* data, size_t size) {
    size = size >= 1024 ? 1023 : size;

    char source[1024] = { 0 };
    memcpy(source, data, size);
    source[size] = '\0';

    TokenArray array = lexer_lex(STR("<fuzzer>"), (Str) { size, source });

    if (array.tokens != NULL) {
        token_array_free(array);
        return 0;
    }

    GrammarTree grammar_tree = parse(array);
    if (grammar_tree.nodes == NULL) {
        return 0;
    }

    TypedAst typed_tree = type_check(grammar_tree);
    if (typed_tree.nodes == NULL) {
        return 0;
    }

    Bytecode code = generate_code(typed_tree);
    if (code.instructions == NULL) {
        return 0;
    }

    JitFunction jitted_function = jit_compile(code, 0);
    if (jitted_function) {
        i64 result = jitted_function();
        return 0;
    } else {
        return 0;
    }

    i64 result = interpret(code);

    return 0;  // Values other than 0 and -1 are reserved for future use.
}

