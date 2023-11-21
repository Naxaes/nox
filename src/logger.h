#pragma once

#include <stdio.h>
#include <stdarg.h>


typedef enum {
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_PANIC
} LogLevel;

typedef struct Logger {
    const char* group;
    LogLevel level;
    size_t   max_size;
    enum {
        LoggerType_File,
        LoggerType_Memory,
    } log_type;
    union {
        FILE* file;
        struct {
            void*  memory;
            size_t size;
        };
    };
} Logger;

Logger logger_make_with_file(const char* group, LogLevel level, FILE* file);
Logger logger_make_with_memory(const char* group, LogLevel level, void* memory, size_t size);

void logger_log(Logger* logger, LogLevel level, const char* file, int line, const char* fmt, ...);
void logger_extend(Logger* logger, LogLevel level, const char* fmt, ...);

#define debug(logger, ...) logger_log(logger, LOG_LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define info(logger, ...)  logger_log(logger, LOG_LEVEL_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define warn(logger, ...)  logger_log(logger, LOG_LEVEL_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define error(logger, ...) logger_log(logger, LOG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define panic(logger, ...) logger_log(logger, LOG_LEVEL_PANIC, __FILE__, __LINE__, __VA_ARGS__)
