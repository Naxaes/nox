#include "lexer/lexer.h"
#include "parser/parser.h"
#include "parser/ast_printer.h"
#include "type_checker/checker.h"
#include "code_generator/generator.h"
#include "code_generator/disassembler.h"
#include "interpreter/interpreter.h"
#include "jit_compiler/jit.h"
#include "transpiler/c_transpiler.h"

#include "str.h"
#include "file.h"
#include "args.h"

#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <locale.h>

typedef struct {
    i64 result;
    int error;
} InterpreterResult;


InterpreterResult run(Str name, Str source, int verbose) {
    TokenArray array = lexer_lex(name, source);
    if (array.tokens == NULL) {
        fprintf(stderr, "Failed to lex source\n");
        return (InterpreterResult) { 0, 1 };
    }

    UntypedAst grammar_tree = parse(array);
    if (grammar_tree.nodes == NULL) {
        fprintf(stderr, "Failed to parse source\n");
        return (InterpreterResult) { 0, 1 };
    }

    ast_print(grammar_tree, stdout);

    TypedAst typed_tree = type_check(grammar_tree);
    if (typed_tree.nodes == NULL) {
        fprintf(stderr, "Failed to type check source\n");
        return (InterpreterResult) { 0, 1 };
    }

    Bytecode code = generate_code(typed_tree);
    if (code.instructions == NULL) {
        fprintf(stderr, "Failed to generate code\n");
        return (InterpreterResult) { 0, 1 };
    }

    fprintf(stdout, "\nBytecode:\n");
    disassemble(code, stdout);
    printf("\n");

    JitFunction jitted_function = jit_compile(code, verbose);
    if (jitted_function) {
        i64 result = jitted_function();
        return (InterpreterResult) { result, 0 };
    } else {
        fprintf(stderr, "[INFO]: Failed to JIT compile\n");
    }

    i64 result = interpret(code);

    compile_c(code);
    return (InterpreterResult) { result, 0 };
}


i64 repl(void) {
    char source[4096] = { 0 };
    char buffer[64] = { 0 };
    size_t source_length = 0;

    while (1) {
        printf("> ");
        fgets(buffer, sizeof(buffer), stdin);

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



int main(int argc, const char* argv[]) {
    setlocale(LC_ALL, "");

    ArgCommands commands = parse_args(argc, argv);

    clock_t start = (commands.take_time) ? clock() : 0;
    switch (commands.mode) {
        case NO_RUN_MODE: {
            fprintf(stderr, "No subcommand provided.");
            printf("%s", USAGE);
            break;
        } case COM: {
            assert(0 && "Not implemented");
            break;
        } case DIS: {
            assert(0 && "Not implemented");
            break;
        } case DOT: {
            assert(0 && "Not implemented");
            break;
        } case REPL: {
            repl();
            break;
        } case RUN: {
            Str source = read_file(commands.input_file);
            if (str_is_empty(source)) {
                fprintf(stderr, "Failed to read file\n");
                return 1;
            }
            if (!commands.is_quiet)
                printf("[INFO]: %s\n%s\n", commands.input_file, source.data);
            InterpreterResult result = run(str_from_c_str(commands.input_file), source, !commands.is_quiet);
            if (result.error) {
                fprintf(stderr, "Failed to run source\n");
                return 1;
            } else {
                printf("[INFO]: Result: %lld\n", result.result);
//                printf("[INFO]: Result: %s\n",  (char*)result.result);
            }
            break;
        } case SIM: {
            assert(0 && "Not implemented");
            break;
        } case HELP: {
            printf("%s", USAGE);
        }
    }

    if (commands.take_time) {
        clock_t stop = clock();
        double time_spent = (double)(stop - start) / CLOCKS_PER_SEC;
        printf("[Finished in %f ms]\n", 1000.0 * time_spent);
    }

    return 0;
}
