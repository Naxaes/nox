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
    HELP,
} RunMode;


typedef struct {
    const char* working_file;
    const char* input_file;
    RunMode mode;
    int is_quiet;
    int take_time;
} ArgCommands;


ArgCommands parse_args(int argc, const char* const argv[]);
