#pragma once

extern const char* USAGE;

typedef enum {
    NO_RUN_MODE,
    COM,
    DIS,
    DOT,
    REPL,
    RUN,
    SIM,
    TRANS,
    HELP,
} RunMode;


typedef struct {
    const char* working_file;
    const char* input_file;
    RunMode mode;
    int verbose;
    int take_time;
    int show_help;
    int as_source;
} ArgCommands;


ArgCommands parse_args(int argc, const char* const argv[]);
