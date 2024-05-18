#include "lib.h"
#include "logger.h"
#include "file.h"
#include "args.h"

#include <stdio.h>
#include <time.h>
#include <locale.h>



int main(int argc, const char* argv[]) {
    setlocale(LC_ALL, "");
    logger_init(LOG_LEVEL_INFO);
    u64 log = logger_gen_group_id("main");

    ArgCommands commands = parse_args(argc, argv);

    clock_t start = (commands.take_time) ? clock() : 0;
    switch (commands.mode) {
        case NO_RUN_MODE: {
            fprintf(stderr, "No subcommand provided.\n");
            fprintf(stderr, "%s\n", USAGE);
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
            if (commands.verbose) {
                fprintf(stdout, "%s\n%s\n", commands.input_file, source.data);
            }

            Str file = str_from_c_str(commands.input_file);
            InterpreterResult result = run(file, source, commands.verbose);
            if (result.error) {
                error(log, "Failed to run source\n");
                return 1;
            } else {
                info(log, "Result: %lld\n", result.result);
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
        fprintf(stdout, "[Finished in %f ms]\n", 1000.0 * time_spent);
    }

    return 0;
}
