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
#include "lib.h"

#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <locale.h>



int main(int argc, const char* argv[]) {
    setlocale(LC_ALL, "");

    /*
     * Command commands[] = {
     *    "build",
     *    "disassemble",
     * };
     * Option options[] = {
     *     { "verbose", 'v', OptionType_Flag },
     *     { "output", 'o', OptionType_String },
     *     { "input", 'i', OptionType_String },
     *     { "help", 'h', OptionType_Flag },
     *  }
     *
     * Command command = parse_command(argc, argv, &commands, &options);
     * switch (command.type) {
     *   case Command_Build: {
     *       Option options = parse_sub_command(command, &commands, &options);
     *       build(options);
     *       break;
     *   } case Command_Disassemble: {
     *       Option options = parse_options(command.options);
     *       disassemble(options);
     *       break;
     *   }
     *  }
     *
     *
     *
     */

    ArgCommands commands = parse_args(argc, argv);

    LogLevel level = commands.verbose ? LOG_LEVEL_DEBUG : LOG_LEVEL_WARN;
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
            InterpreterResult result;
            if (commands.as_source) {
                result = run_from_source(STR("<source>"), str_from_c_str(commands.input_file), &main_logger);
            } else {
                result = run_from_file(str_from_c_str(commands.input_file), &main_logger);
            }

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
            c_transpile_from_source(str_from_c_str(commands.input_file), source, &main_logger);
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
