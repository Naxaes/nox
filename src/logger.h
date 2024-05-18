#pragma once

#include "preamble.h"

#include <stdio.h>
#include <execinfo.h>
#include <signal.h>

// If allocator is included before logger, then this will
// cause realloc to be redefined. Include this before allocator.h.
#include <stdlib.h>
#undef assert



/* ---- DEFAULT ARGUMENTS ----
Wrap the function in a variadic macro that calls to WITH_DEFAULTS with a
name and __VA_ARGS__. For each parameter passed in it'll now dispatch
to macros (or functions) named "nameX", where X is the number of parameters.

    // Define the dispatcher.
    #define greet(...) WITH_DEFAULTS(greet, __VA_ARGS__)

    // Define the overloaded functions (or function-like macros) you want.
    #define greet1(name)           printf("%s %s!", "Hello", name)
    #define greet2(greeting, name) printf("%s %s!", greeting, name)

    // Call.
    greet("Sailor");                      // printf("%s %s!", "Hello",     "Sailor");
    greet("Greetings", "Sailor");         // printf("%s %s!", "Greetings", "Sailor");
    greet("Greetings", "Sailor", "!!!");  // Error: greet3 is not defined.

This is restricted to a minimum of 1 argument and a maximum of 8.
*/
#define CONCATENATE(a, b) a##b

#define POP_10TH_ARG(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, argN, ...) argN
#define VA_ARGS_COUNT(...)  POP_10TH_ARG(__VA_ARGS__, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define SELECT_FUNCTION(function, postfix) CONCATENATE(function, postfix)
#define WITH_DEFAULTS(f, ...) SELECT_FUNCTION(f, VA_ARGS_COUNT(__VA_ARGS__))(__VA_ARGS__)



typedef struct Source_Location {
    const char* file;
    const char* function;
    int line;
} Source_Location;
#define CURRENT_SOURCE_LOCATION (Source_Location){ __FILE__, __FUNCTION_NAME__, __LINE__ }


typedef enum Log_Level {
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_ASSERT,
    LOG_LEVEL_PANIC,
    LOG_LEVEL_LAST = LOG_LEVEL_PANIC
} Log_Level;


void logger_init(Log_Level level);
void logger_log(Log_Level level, u64 group, Source_Location source_location, const char* message, va_list args);
static inline int __attribute__((noinline, noreturn)) terminate_with_backtrace(void);


extern Log_Level LOG_LEVEL;
extern FILE* LOG_OUTPUT[LOG_LEVEL_LAST+1];
extern u64 LOG_ENABLED_GROUPS;
extern const char* LOG_GROUP_NAMES[64];


static inline void logger_set_output(Log_Level level, FILE* file) {
    LOG_OUTPUT[level] = file;
}

static inline void logger_set_level(Log_Level level) {
    LOG_LEVEL = level;
}

static inline void logger_enable_group(u64 group_id) {
    LOG_ENABLED_GROUPS |= (1 << group_id);
}

static inline bool logger_is_group_enabled(u64 group_id) {
    return LOG_ENABLED_GROUPS & (1 << group_id);
}

static inline u64 logger_gen_group_id(const char* name) {
    static u64 group_id = 1;
    if (group_id > 64) {
        logger_log(LOG_LEVEL_PANIC, 0, CURRENT_SOURCE_LOCATION, "Couldn't generate id. Too many log groups", 0);
        terminate_with_backtrace();
    }
    LOG_GROUP_NAMES[group_id] = name;
    logger_enable_group(group_id);
    return group_id++;
}


#define debug(id, ...)  logger_debug(id,  CURRENT_SOURCE_LOCATION, __VA_ARGS__)
static inline void __attribute__((always_inline))
logger_debug(u64 group, Source_Location source_location, const char* message, ...) {
    if (LOG_LEVEL <= LOG_LEVEL_DEBUG && logger_is_group_enabled(group)) {
        va_list args;
        va_start(args, message);
        logger_log(LOG_LEVEL_DEBUG, group, source_location, message, args);
        va_end(args);
    }
}

#define info(log, ...)      logger_info(log,  CURRENT_SOURCE_LOCATION, __VA_ARGS__)
#define infol(id, ...) logger_info(id, CURRENT_SOURCE_LOCATION, __VA_ARGS__)
static inline void __attribute__((always_inline))
logger_info(u64 group, Source_Location source_location, const char* message, ...) {
    if (LOG_LEVEL <= LOG_LEVEL_INFO && logger_is_group_enabled(group)) {
        va_list args;
        va_start(args, message);
        logger_log(LOG_LEVEL_INFO, group, source_location, message, args);
        va_end(args);
    }
}

#define warn(...) logger_warning(0, CURRENT_SOURCE_LOCATION, __VA_ARGS__)
static inline void __attribute__((always_inline))
logger_warning(u64 group, Source_Location source_location, const char* message, ...) {
    if (LOG_LEVEL <= LOG_LEVEL_WARNING && logger_is_group_enabled(group)) {
        va_list args;
        va_start(args, message);
        logger_log(LOG_LEVEL_WARNING, group, source_location, message, args);
        va_end(args);
    }
}

#define error(log, ...) logger_error(log, CURRENT_SOURCE_LOCATION, __VA_ARGS__)
static inline void __attribute__((always_inline))
logger_error(u64 group, Source_Location source_location, const char* message, ...) {
    if (LOG_LEVEL <= LOG_LEVEL_ERROR && logger_is_group_enabled(group)) {
        va_list args;
        va_start(args, message);
        logger_log(LOG_LEVEL_ERROR, group, source_location, message, args);
        va_end(args);
    }
}


#define assert(...) WITH_DEFAULTS(assert, __VA_ARGS__)
#define assert1(cond)           ((cond) ? 0 : logger_assert(0, CURRENT_SOURCE_LOCATION, "Assertion failed: %s", #cond) && terminate_with_backtrace())
#define assert2(cond, msg)      ((cond) ? 0 : logger_assert(0, CURRENT_SOURCE_LOCATION, "Assertion failed: %s (" msg ")", #cond) && terminate_with_backtrace())
#define assert3(cond, msg, ...) ((cond) ? 0 : logger_assert(0, CURRENT_SOURCE_LOCATION, "Assertion failed: %s (" msg ")", #cond, __VA_ARGS__) && terminate_with_backtrace())
#define assert4(cond, msg, ...) ((cond) ? 0 : logger_assert(0, CURRENT_SOURCE_LOCATION, "Assertion failed: %s (" msg ")", #cond, __VA_ARGS__) && terminate_with_backtrace())
#define assert5(cond, msg, ...) ((cond) ? 0 : logger_assert(0, CURRENT_SOURCE_LOCATION, "Assertion failed: %s (" msg ")", #cond, __VA_ARGS__) && terminate_with_backtrace())
#define assert6(cond, msg, ...) ((cond) ? 0 : logger_assert(0, CURRENT_SOURCE_LOCATION, "Assertion failed: %s (" msg ")", #cond, __VA_ARGS__) && terminate_with_backtrace())
#define assert7(cond, msg, ...) ((cond) ? 0 : logger_assert(0, CURRENT_SOURCE_LOCATION, "Assertion failed: %s (" msg ")", #cond, __VA_ARGS__) && terminate_with_backtrace())
#define assert8(cond, msg, ...) ((cond) ? 0 : logger_assert(0, CURRENT_SOURCE_LOCATION, "Assertion failed: %s (" msg ")", #cond, __VA_ARGS__) && terminate_with_backtrace())
#define assert9(cond, msg, ...) ((cond) ? 0 : logger_assert(0, CURRENT_SOURCE_LOCATION, "Assertion failed: %s (" msg ")", #cond, __VA_ARGS__) && terminate_with_backtrace())
static inline int __attribute__((always_inline))
logger_assert(u64 group, Source_Location source_location, const char* message, ...) {
    if (LOG_LEVEL <= LOG_LEVEL_ASSERT && logger_is_group_enabled(group)) {
        va_list args;
        va_start(args, message);
        logger_log(LOG_LEVEL_ASSERT, group,  source_location, message, args);
        va_end(args);

        return 1;
    }

    return 0;
}

#define panic(log, ...) logger_panic(log, CURRENT_SOURCE_LOCATION, __VA_ARGS__)
static inline void __attribute__((always_inline,  noreturn))
logger_panic(u64 group, Source_Location source_location, const char* message, ...) {
    if (LOG_LEVEL <= LOG_LEVEL_PANIC && logger_is_group_enabled(group)) {
        va_list args;
        va_start(args, message);
        logger_log(LOG_LEVEL_PANIC, group, source_location, message, args);
        va_end(args);
    }

    terminate_with_backtrace();
}




static inline int __attribute__((noinline, noreturn)) terminate_with_backtrace(void) {
    void* callstack[128] = { 0 };
    int frames = backtrace(callstack, 128);
    char** strs = backtrace_symbols(callstack, frames);
    for (int i = 0; i < frames-1; ++i) {
        fprintf(stderr, "%s\n", strs[i]);
    }
    free(strs);

    raise(SIGTRAP);
    exit(EXIT_FAILURE);
}

#ifdef LOGGER_IMPLEMENTATION
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
#endif // LOGGER_IMPLEMENTATION
