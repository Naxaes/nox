#include "logger.h"
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
#include <locale.h>


#define LOGGER_IMPLEMENTATION
#include "logger.h"


Log_Level LOG_LEVEL;
FILE* LOG_OUTPUT[LOG_LEVEL_LAST+1];
u64 LOG_ENABLED_GROUPS;
const char* LOG_GROUP_NAMES[64];

void logger_init(Log_Level level) {
    LOG_OUTPUT[LOG_LEVEL_DEBUG]   = stdout;
    LOG_OUTPUT[LOG_LEVEL_INFO]    = stdout;
    LOG_OUTPUT[LOG_LEVEL_WARNING] = stderr;
    LOG_OUTPUT[LOG_LEVEL_ERROR]   = stderr;
    LOG_OUTPUT[LOG_LEVEL_ASSERT]  = stderr;
    LOG_OUTPUT[LOG_LEVEL_PANIC]   = stderr;

    LOG_GROUP_NAMES[0] = "default";
    LOG_ENABLED_GROUPS = 1;
    LOG_LEVEL = level;
}

void logger_log(Log_Level level, u64 group, Source_Location source_location, const char* message, va_list args) {
    static const char* log_level_strings[] = {
            [LOG_LEVEL_DEBUG]   = "debug",
            [LOG_LEVEL_INFO]    = "info",
            [LOG_LEVEL_WARNING] = "warning",
            [LOG_LEVEL_ERROR]   = "error",
            [LOG_LEVEL_ASSERT]  = "assert",
            [LOG_LEVEL_PANIC]   = "panic",
    };

    void* file = LOG_OUTPUT[level];
    if ((uintptr_t) file == 0) {
        file = stdout;
    } else if ((uintptr_t) file == 1) {
        file = stderr;
    }

    fprintf(file, "[%s | %s]: %s:%d: ", log_level_strings[level], LOG_GROUP_NAMES[group], source_location.file, source_location.line);
    vfprintf(file, message, args);
    fprintf(file, "\n");
}



typedef struct {
    i64 result;
    int error;
} InterpreterResult;


int c_transpile(Str name, Str source, int verbose) {
    TokenArray array = lexer_lex(name, source);
    if (array.tokens == NULL) {
        fprintf(stderr, "Failed to lex source\n");
        return -1;
    }

    GrammarTree grammar_tree = parse(array);
    if (grammar_tree.nodes == NULL) {
        fprintf(stderr, "Failed to parse source\n");
        return -1;
    }

    if (verbose)
        ast_print(grammar_tree, stdout);

    TypedAst typed_tree = type_check(grammar_tree);
    if (typed_tree.nodes == NULL) {
        fprintf(stderr, "Failed to type check source\n");
        return -1;
    }

    Bytecode code = generate_code(typed_tree);
    if (code.instructions == NULL) {
        fprintf(stderr, "Failed to generate code\n");
        return -1;
    }

    compile_c(code);

    return 0;
}


InterpreterResult run(Str name, Str source, int verbose) {
    TokenArray array = lexer_lex(name, source);
    if (array.tokens == NULL) {
        fprintf(stderr, "Failed to lex source\n");
        return (InterpreterResult) { 0, 1 };
    }

    GrammarTree grammar_tree = parse(array);
    if (grammar_tree.nodes == NULL) {
        fprintf(stderr, "Failed to parse source\n");
        return (InterpreterResult) { 0, 1 };
    }

    if (verbose)
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

    if (verbose) {
        fprintf(stdout, "\nBytecode:\n");
        disassemble(code, stdout);
        printf("\n");
    }

    JitFunction jitted_function = jit_compile(code, verbose);
    if (jitted_function) {
        i64 result = jitted_function();
        return (InterpreterResult) { result, 0 };
    } else {
        fprintf(stderr, "[INFO]: Failed to JIT compile\n");
    }

    i64 result = interpret(code);

    return (InterpreterResult) { result, 0 };
}


i64 repl(void) {
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



int main(int argc, const char* argv[]) {
    setlocale(LC_ALL, "");
    logger_init(LOG_LEVEL_INFO);
    u64 log = logger_gen_group_id("main");

    ArgCommands commands = parse_args(argc, argv);

    clock_t start = (commands.take_time) ? clock() : 0;
    switch (commands.mode) {
        case NO_RUN_MODE: {
            error(log, "No subcommand provided.");
            printf("%s", USAGE);
            break;
        } case COM: {
            panic(log, "Not implemented");
            break;
        } case DIS: {
            panic(log, "Not implemented");
            break;
        } case DOT: {
            panic(log, "Not implemented");
            break;
        } case REPL: {
            repl();
            break;
        } case RUN: {
            Str source = read_file(commands.input_file);
            if (str_is_empty(source)) {
                error(log, "Failed to read file\n");
                return 1;
            }
            if (commands.verbose)
                infol(log, "%s\n%s\n", commands.input_file, source.data);
            InterpreterResult result = run(str_from_c_str(commands.input_file), source, commands.verbose);
            if (result.error) {
                error(log, "Failed to run source\n");
                return 1;
            } else {
                infol(log, "Result: %lld\n", result.result);
            }
            break;
        } case SIM: {
            panic(log, "Not implemented");
            break;
        } case TRANS: {
            Str source = read_file(commands.input_file);
            if (str_is_empty(source)) {
                error(log, "Failed to read file\n");
                return 1;
            }
            c_transpile(str_from_c_str(commands.input_file), source, commands.verbose);
        } break;
        case HELP: {
            printf("%s", USAGE);
        }
    }

    if (commands.take_time) {
        clock_t stop = clock();
        double time_spent = (double)(stop - start) / CLOCKS_PER_SEC;
        infol(log, "[Finished in %f ms]\n", 1000.0 * time_spent);
    }

    return 0;
}
