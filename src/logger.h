#include <stdio.h>
#include <stdarg.h>


#define stringify_(x) #x
#define stringify(x) stringify_(x)

#define info(...) logger_log(LOG_LEVEL_INFO, "[INFO]: " __FILE__ ":" stringify(__LINE__) ": ", __VA_ARGS__)
// #define warn(...) logger_log(LOG_LEVEL_WARN, "[WARN]: " __FILE__ ":" stringify(__LINE__) ": ", __VA_ARGS__)
// #define error(...) logger_log(LOG_LEVEL_ERROR, "[ERROR]: " __FILE__ ":" stringify(__LINE__) ": ", __VA_ARGS__)


typedef enum {
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
} LogLevel;


static inline void logger_log(LogLevel level, const char* fmt, ...) {
    (void) level;
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
}
