#include "args.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


const char* USAGE = ""
"Usage: nox [OPTIONS] <SUBCOMMAND> [ARGS]\n"
"  OPTIONS:\n"
"    -q, --quiet       Don't output anything from the compiler\n"
"    -t, --time        Output time to finish command\n"
"    -h, --help        Display options for a command\n"
"  SUBCOMMAND:\n"
"    com  [file]       Compile the project or a given file\n"
"    dis  <file>       Disassemble a file\n"
"    dot  <file>       Generates a dot Graphviz file\n"
"    repl              Start the interactive session\n"
"    run  <file>       Run a file\n"
"    sim  <file>       Interpret a file\n"
"    help              Show this output\n";

const char* RUN_MODE_STRING[] = {
        [NO_RUN_MODE] = "none",
        [COM]  = "com",
        [DIS]  = "dis",
        [DOT]  = "dot",
        [REPL] = "repl",
        [RUN]  = "run",
        [SIM]  = "sim",
        [HELP] = "help",
};

#define is_argument(s, x)  (memcmp(s, x, strlen(x)) == 0)


int parse_build(int argc, const char* const argv[], ArgCommands* commands) {
    if (commands->mode == NO_RUN_MODE) {
        commands->mode = COM;
    } else {
        fprintf(stderr, "'%s' is a top-level subcommand, not a subcommand for '%s'. They can't be run at the same time.\n", RUN_MODE_STRING[COM], RUN_MODE_STRING[commands->mode]);
        exit(EXIT_FAILURE);
    }
    if (argc > 0) {
        commands->input_file = argv[0];
        return 1;
    }
    return 0;
}

int parse_dis(int argc, const char* const argv[], ArgCommands* commands) {
    if (commands->mode == NO_RUN_MODE) {
        commands->mode = DIS;
    } else {
        fprintf(stderr, "'%s' is a top-level subcommand, not a subcommand for '%s'. They can't be run at the same time.\n", RUN_MODE_STRING[DIS], RUN_MODE_STRING[commands->mode]);
        exit(EXIT_FAILURE);
    }
    if (argc > 0) {
        commands->input_file = argv[0];
        return 1;
    }
    return 0;
}

int parse_dot(int argc, const char* const argv[], ArgCommands* commands) {
    if (commands->mode == NO_RUN_MODE) {
        commands->mode = DOT;
    } else {
        fprintf(stderr, "'%s' is a top-level subcommand, not a subcommand for '%s'. They can't be run at the same time.\n", RUN_MODE_STRING[DOT], RUN_MODE_STRING[commands->mode]);
        exit(EXIT_FAILURE);
    }
    if (argc > 0) {
        commands->input_file = argv[0];
        return 1;
    }
    return 0;
}

int parse_repl(int argc, const char* const argv[], ArgCommands* commands) {
    (void)(argc);
    (void)(argv);
    if (commands->mode == NO_RUN_MODE) {
        commands->mode = REPL;
    } else {
        fprintf(stderr, "'%s' is a top-level subcommand, not a subcommand for '%s'. They can't be run at the same time.\n", RUN_MODE_STRING[REPL], RUN_MODE_STRING[commands->mode]);
        exit(EXIT_FAILURE);
    }
    return 0;
}

int parse_run(int argc, const char* const argv[], ArgCommands* commands) {
    if (commands->mode == NO_RUN_MODE) {
        commands->mode = RUN;
    } else {
        fprintf(stderr, "'%s' is a top-level subcommand, not a subcommand for '%s'. They can't be run at the same time.\n", RUN_MODE_STRING[RUN], RUN_MODE_STRING[commands->mode]);
        exit(EXIT_FAILURE);
    }
    if (argc > 0) {
        commands->input_file = argv[0];
        return 1;
    } else {
        fprintf(stderr, "'%s' requires an input file.\n", RUN_MODE_STRING[RUN]);
        exit(EXIT_FAILURE);
    }
    return 0;
}

int parse_sim(int argc, const char* const argv[], ArgCommands* commands) {
    if (commands->mode == NO_RUN_MODE) {
        commands->mode = SIM;
    } else {
        fprintf(stderr, "'%s' is a top-level subcommand, not a subcommand for '%s'. They can't be run at the same time.\n", RUN_MODE_STRING[SIM], RUN_MODE_STRING[commands->mode]);
        exit(EXIT_FAILURE);
    }
    if (argc > 0) {
        commands->input_file = argv[0];
        return 1;
    }
    return 0;
}



ArgCommands parse_args(int argc, const char* const argv[]) {
    ArgCommands commands = { .working_file=argv[0], .input_file=0, .mode=NO_RUN_MODE, .verbose=0, .take_time=0 };
    argv++; argc--;
    for (int i = 0; i < argc; ++i) {
        const char* const arg = argv[i];
        if      (is_argument(arg, RUN_MODE_STRING[COM]))   {  i += parse_build(argc-i-1, argv+i+1, &commands); }
        else if (is_argument(arg, RUN_MODE_STRING[DIS]))   {  i += parse_dis(argc-i-1,   argv+i+1, &commands); }
        else if (is_argument(arg, RUN_MODE_STRING[DOT]))   {  i += parse_dot(argc-i-1,   argv+i+1, &commands); }
        else if (is_argument(arg, RUN_MODE_STRING[REPL]))  {  i += parse_repl(argc-i-1,  argv+i+1, &commands); }
        else if (is_argument(arg, RUN_MODE_STRING[RUN]))   {  i += parse_run(argc-i-1,   argv+i+1, &commands); }
        else if (is_argument(arg, RUN_MODE_STRING[SIM]))   {  i += parse_sim(argc-i-1,   argv+i+1, &commands); }
        else if (is_argument(arg, RUN_MODE_STRING[HELP]))  {  commands.mode = HELP; }
        else if (is_argument(arg, "-v") || is_argument(arg, "--verbose")) {  commands.verbose   = 1; }
        else if (is_argument(arg, "-t") || is_argument(arg, "--time"))    {  commands.take_time = 1; }
        else if (is_argument(arg, "-h") || is_argument(arg, "--help"))    {  commands.show_help = 1; }
        else if (is_argument(arg, "-s") || is_argument(arg, "--source"))  {  commands.as_source = 1; }
        else {
            // @TODO: Check that there are no more commands.
            fprintf(stderr, "Unknown command '%s'\n", argv[i]);
            printf("%s", USAGE);
            exit(EXIT_SUCCESS);
        }
    }

    return commands;
}
