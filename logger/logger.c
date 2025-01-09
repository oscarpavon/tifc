#include "logger.h"

#include <stdio.h>
#include <stdarg.h>


logger_status_t logger_init(logger_t *const logger,
        const severity_t severity,
        log_path_t path)
{
    FILE *output = fopen(path, "a");
    if (!output) return LOGGER_FAIL;

    *logger = (logger_t){
        .severity = severity,
        .output = output,
    };

    return LOGGER_SUCCESS;
}


logger_status_t logger_log(logger_t *restrict const logger,
        const severity_t severity,
        const char *restrict format,
        ...)
{
    if (severity < logger->severity) return LOGGER_SKIPPED;

    va_list list;
    va_start(list, format);
#ifdef ESCAPE_COLORS
#ifndef ESC
#   define ESC "\x1b"
#endif

    static const char *const EscapeColors[] = {
        [LOGGER_INFO]     = ESC "[0m",
        [LOGGER_DEBUG]    = ESC "[36m",
        [LOGGER_WARNING]  = ESC "[33m",
        [LOGGER_CRITICAL] = ESC "[91m"
    };
    fprintf(logger->output, "%s", EscapeColors[severity]);
#endif
    int bytes = vfprintf(logger->output, format, list);
    if (bytes < 0)
    {
        perror("logger");
        return LOGGER_FAIL;
    }
#ifdef ESCAPE_COLORS
    fprintf(logger->output, "%s", EscapeColors[0]);
#endif
    fflush(logger->output);
    va_end(list);
    return LOGGER_SUCCESS;
}

void logger_deinit(logger_t *const logger)
{
    fclose(logger->output);
}

