#include <stdio.h>

#include "../include/logger.h"


void logger_log(enum log_level level, const char *file, int line, const char *format, ...)
{
    const char *level_string;
    va_list args;

    switch (level) {
    case LOG_LEVEL_ERROR: level_string = "ERROR";
                          break;

    case LOG_LEVEL_WARN: level_string = "WARN";
                          break;

    case LOG_LEVEL_INFO: level_string = "INFO";
                          break;

    case LOG_LEVEL_DEBUG: level_string = "DEBUG";
                          break;
    default:  level_string = "UNKNOWN";
              break;
    }

    fprintf(stderr, "[%s] %s:%d: ", level_string, file, line);

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    fprintf(stderr, "\n");
}