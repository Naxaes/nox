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


typedef struct {
    i64 result;
    int error;
} InterpreterResult;


int c_transpile(Str name, Str source, Logger* logger) {
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

    TypedAst typed_tree = type_check(grammar_tree, logger);
    if (typed_tree.nodes == NULL) {
        error(logger, "Failed to type check source\n");
        return -1;
    }

    Bytecode code = generate_code(typed_tree);
    if (code.instructions == NULL) {
        error(logger, "Failed to generate code\n");
        return -1;
    }

    compile_c(code);

    return 0;
}


InterpreterResult run(Str name, Str source, Logger* logger) {
    TokenArray array = lexer_lex(name, source, logger);
    if (array.tokens == NULL) {
        error(logger, "Failed to lex source\n");
        return (InterpreterResult) { 0, 1 };
    }

    GrammarTree grammar_tree = parse(array, logger);
    if (grammar_tree.nodes == NULL) {
        error(logger, "Failed to parse source\n");
        return (InterpreterResult) { 0, 1 };
    }

    if (logger->level == LOG_LEVEL_DEBUG)
        ast_print(grammar_tree, stdout);

    TypedAst typed_tree = type_check(grammar_tree, logger);
    if (typed_tree.nodes == NULL) {
        error(logger, "Failed to type check source\n");
        return (InterpreterResult) { 0, 1 };
    }

    Bytecode code = generate_code(typed_tree);
    if (code.instructions == NULL) {
        error(logger, "Failed to generate code\n");
        return (InterpreterResult) { 0, 1 };
    }

    if (logger->level == LOG_LEVEL_DEBUG) {
        fprintf(stdout, "\nBytecode:\n");
        disassemble(code, stdout);
        printf("\n");
    }

    JitFunction jitted_function = jit_compile(code, 1);
    if (jitted_function) {
        i64 result = jitted_function();
        return (InterpreterResult) { result, 0 };
    } else {
        error(logger, "[INFO]: Failed to JIT compile\n");
    }

    i64 result = interpret(code);

    return (InterpreterResult) { result, 0 };
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

        InterpreterResult result = run(STR("<repl>"), (Str) { source_length, source }, &logger);
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

    LogLevel level = commands.verbose ? LOG_LEVEL_INFO : LOG_LEVEL_WARN;
    Logger main_logger = logger_make_with_file("Main", level, stdout);

    clock_t start = (commands.take_time) ? clock() : 0;
    switch (commands.mode) {
        case NO_RUN_MODE: {
            error(&main_logger, "No subcommand provided.");
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
                error(&main_logger, "Failed to read file\n");
                return 1;
            }
            if (commands.verbose)
                info(&main_logger, "%s\n%s\n", commands.input_file, source.data);

            InterpreterResult result = run(str_from_c_str(commands.input_file), source, &main_logger);
            if (result.error) {
                error(&main_logger, "Failed to run source\n");
                return 1;
            } else {
                info(&main_logger, "Result: %lld\n", result.result);
            }
            break;
        } case SIM: {
            assert(0 && "Not implemented");
            break;
        } case TRANS: {
            Str source = read_file(commands.input_file);
            if (str_is_empty(source)) {
                error(&main_logger, "Failed to read file\n");
                return 1;
            }
            c_transpile(str_from_c_str(commands.input_file), source, &main_logger);
        } break;
        case HELP: {
            printf("%s", USAGE);
        }
    }

    if (commands.take_time) {
        clock_t stop = clock();
        double time_spent = (double)(stop - start) / CLOCKS_PER_SEC;
        info(&main_logger, "[Finished in %f ms]\n", 1000.0 * time_spent);
    }

    return 0;
}
