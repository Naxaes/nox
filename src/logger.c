#include "logger.h"
#include <stdarg.h>
#include <stdlib.h>


static const char* LOG_LEVEL_NAMES[] = {
    [LOG_LEVEL_DEBUG]   = "DEBUG",
    [LOG_LEVEL_INFO]    = "INFO",
    [LOG_LEVEL_WARN]    = "WARN",
    [LOG_LEVEL_ERROR]   = "ERROR",
    [LOG_LEVEL_PANIC]   = "PANIC",
    [LOG_LEVEL_NONE]    = "NONE",
};


Logger logger_make_with_file(const char* group, LogLevel level, FILE* file) {
    return (Logger){
        .group = group,
        .level = level,
        .log_type = LoggerType_File,
        .file = file,
    };
}

Logger logger_make_with_memory(const char* group, LogLevel level, void* memory, size_t size) {
    return (Logger){
        .group = group,
        .level = level,
        .log_type = LoggerType_Memory,
        .memory = memory,
        .size = 0,
        .max_size = size,
    };
}

void logger_extend(Logger* logger, LogLevel level, const char* fmt, ...) {
    if (logger->level > level) {
        return;
    }

    if (logger->log_type == LoggerType_File) {
        va_list args;
        va_start(args, fmt);
        vfprintf(logger->file, fmt, args);
        va_end(args);
    } else {
        int bytes_left = (int)logger->max_size - (int)logger->size;
        if (bytes_left <= 0) {
            goto end;
        }

        va_list args;
        va_start(args, fmt);
        bytes_left -= vsnprintf(logger->memory, bytes_left, fmt, args);
        va_end(args);

        logger->size = logger->max_size - bytes_left;
    }

    end:;
    if (logger->level == LOG_LEVEL_PANIC)
        exit(EXIT_FAILURE);
}

void logger_log(Logger* logger, LogLevel level, const char* file, int line, const char* fmt, ...) {
    if (logger->level > level) {
        return;
    }

    if (logger->log_type == LoggerType_File) {
        fprintf(logger->file, "[%s] (%s) %s:%d: ", LOG_LEVEL_NAMES[level], logger->group, file, line);
        va_list args;
        va_start(args, fmt);
        vfprintf(logger->file, fmt, args);
        va_end(args);
    } else {
        int bytes_left = (int)logger->max_size - (int)logger->size;
        if (bytes_left <= 0) {
            goto end;
        }

        bytes_left -= snprintf(logger->memory, bytes_left, "[%s] (%s) %s:%d: ", LOG_LEVEL_NAMES[level], logger->group, file, line);

        va_list args;
        va_start(args, fmt);
        bytes_left -= vsnprintf(logger->memory, bytes_left, fmt, args);
        va_end(args);

        logger->size = logger->max_size - bytes_left;
    }

    end:;
    if (logger->level == LOG_LEVEL_PANIC)
        exit(EXIT_FAILURE);
}
